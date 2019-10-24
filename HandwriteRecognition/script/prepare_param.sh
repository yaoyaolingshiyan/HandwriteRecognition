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
presenter_view_app_name=$2
data_source=$3

. ${script_path}/func_util.sh
. ${script_path}/func_deploy.sh

app_path="${script_path}/.."

function main()
{
    if [[ $# -lt 3 ]];then
        echo "ERROR: invalid command, please check your command format: ./prepare_param.sh host_ip presenter_view_app_name camera_channel_name."
        exit 1
    fi
    check_ip_addr ${remote_host}
    if [[ $? -ne 0 ]];then
        echo "ERROR: invalid host ip, please check your command format: ./prepare_param.sh host_ip presenter_view_app_name camera_channel_name."
        exit 1
    fi

    if [[ ${data_source} != "Channel-1" && ${data_source} != "Channel-2" ]];then
        echo "ERROR: invalid camera channel name, please input Channel-1 or Channel-2."
        exit 1
    fi
    echo "Prepare app configuration..."
    cp -r ${app_path}/handwriteapp/graph_deploy.config ${app_path}/handwriteapp/out/graph.config
    sed -i "s/\${template_data_source}/${data_source}/g" ${app_path}/handwriteapp/out/graph.config
    sed -i "s/\${template_app_name}/${presenter_view_app_name}/g" ${app_path}/handwriteapp/out/graph.config
    
    parse_remote_port
    
    upload_file ${app_path}/handwriteapp/out/graph.config "~/HIAI_PROJECTS/ascend_workspace/handwriteapp/out"
    if [[ $? -ne 0 ]];then
        echo "ERROR: sync ${app_path}/handwriteapp/graph.config ${remote_host}:./HIAI_PROJECTS/ascend_workspace/handwriteapp/out failed, please check /var/log/syslog for details."
        exit 1
    fi
    echo "Finish to prepare handwriteapp params."
    exit 0
}

main $*
