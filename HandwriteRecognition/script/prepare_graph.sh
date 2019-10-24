#!/bin/bash
#
#   =======================================================================
#
# Copyright (C) 2018, Hisilicon Technologies Co., Ltd. All Rights Reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   1 Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
#   2 Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
#   3 Neither the names of the copyright holders nor the names of the
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#   =======================================================================

# ************************Variable*********************************************

script_path="$( cd "$(dirname "$0")" ; pwd -P )"

remote_host=$1
download_mode=$2

. ${script_path}/func_deploy.sh
. ${script_path}/func_util.sh

app_path="${script_path}/.."

function modify_graph()
{
    echo "Modify presenter server information in graph.config..."
    cp -r ${app_path}/handwriteapp/graph_template.config ${app_path}/handwriteapp/graph_deploy.config
    presenter_ip=`grep presenter_server_ip ${app_path}/presenterserver/hand_write/config/config.conf | awk -F '=' '{print $2}' | sed 's/[^0-9.]//g'`
    if [[ $? -ne 0 ]];then
        echo "ERROR: get presenter server ip failed, please check ${app_path}/presenterserver/hand_write/config/config.conf."
        return 1
    fi
    
    presenter_port=`grep presenter_server_port ${app_path}/presenterserver/hand_write/config/config.conf | awk -F '=' '{print $2}' | sed 's/[^0-9]//g'`
    if [[ $? -ne 0 ]];then
        echo "ERROR: get presenter server port failed, please check ${app_path}/presenterserver/hand_write/config/config.conf."
        return 1
    fi
    
    sed -i "s/\${template_presenter_ip}/${presenter_ip}/g" ${app_path}/handwriteapp/graph_deploy.config
    sed -i "s/\${template_presenter_port}/${presenter_port}/g" ${app_path}/handwriteapp/graph_deploy.config
    return 0
}

function main()
{
    echo "Modify presenter server configuration..."
    check_ip_addr ${remote_host}
    if [[ $? -ne 0 ]];then
        echo "ERROR: invalid host ip, please check your command format: ./prepare_graph.sh host_ip [download_mode(local/internet)]."
        exit 1
    fi
    bash ${script_path}/prepare_presenter_server.sh ${remote_host} ${download_mode}
    if [[ $? -ne 0 ]];then
        exit 1
    fi
    
    modify_graph
    if [[ $? -ne 0 ]];then
        exit 1
    fi
    
    echo "Finish to prepare handwriteapp graph."
    exit 0
}

main
