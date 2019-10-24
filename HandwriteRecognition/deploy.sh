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

app_name="handwriteapp"

. ${script_path}/script/func_deploy.sh
. ${script_path}/script/func_util.sh

function deploy_app()
{
    #set remote_port
    parse_remote_port

    #build common
    echo "[Step] Build common libs..."
    bash ${script_path}/script/build_ezdvpp.sh ${download_mode}
    if [[ $? -ne 0 ]];then
        return 1
    fi

    bash ${script_path}/script/build_presenteragent.sh ${download_modes}
    if [[ $? -ne 0 ]];then
        return 1
    fi

    #build app
    echo "[Step] Build app libs..."
    bash ${script_path}/script/build.sh
    if [[ $? -ne 0 ]];then
        return 1
    fi

    #prepare_model.sh: download_mode
    if [[ ${download_mode} != "none" ]];then
        echo "[Step] Prepare models..."
        if [[ ${download_mode} == "local" ]];then
            model_version="local"
        else
            model_version="internet"
            #model_version=`grep VERSION ${DDK_HOME}/ddk_info | awk -F '"' '{print $4}'`
            #if [[ $? -ne 0 ]];then
                #echo "ERROR: can not get version in ${DDK_HOME}/ddk_info, please check your env."
                #return 1
            #fi
        fi
        bash ${script_path}/script/prepare_model.sh ${model_version}
        if [[ $? -ne 0 ]];then
            return 1
        fi
    fi

    #deploy common libs
    echo "[Step] Deploy common libs..."
    bash ${script_path}/script/deploy_sdk.sh ${remote_host}
    if [[ $? -ne 0 ]];then
        return 1
    fi

    #deploy dataset
    if [ -d ${script_path}/MyDataset ];then
        echo "[Step] Deploy dataset..."
        upload_path ${script_path}/MyDataset "~/HIAI_DATANDMODELSET/ascend_workspace"
        if [[ $? -ne 0 ]];then
            return 1
        fi
    fi

    #deploy models
    if [ -d ${script_path}/MyModel ];then
        echo "[Step] Deploy models..."
        upload_path ${script_path}/MyModel "~/HIAI_DATANDMODELSET/ascend_workspace" "true"
        if [[ $? -ne 0 ]];then
            return 1
        fi
    fi

    if [ -d ${script_path}/${app_name}/out ];then
        echo "[Step] Deploy app libs..."
        deploy_app_lib_path ${app_name} ${script_path} ${remote_host}
        if [[ $? -ne 0 ]];then
            return 1
        fi
    fi
    return 0
}

main()
{
    check_ip_addr ${remote_host}
    if [[ $? -ne 0 ]];then
        echo "ERROR: invalid host ip, please check your command format: ./deploy.sh host_ip [download_mode(local/internet)]."
        exit 1
    fi
    
    deploy_app
    if [[ $? -ne 0 ]];then
        exit 1
    fi
    
    echo "[Step] Prepare presenter server information and graph.confg..."
    bash ${script_path}/script/prepare_graph.sh ${remote_host} ${download_mode}
    echo "Finish to deploy handwriteapp."
    exit 0
}

main
#end
