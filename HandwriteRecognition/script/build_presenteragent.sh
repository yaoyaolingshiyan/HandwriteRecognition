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

download_mode=$1
presenteragent_version="1.1.2"

function download_code()
{
    if [ -d ${script_path}/presenteragent ];then
        echo "Presenteragent code is found..."
        return 0
    else
        if [[ ${download_mode} == "local" ]];then
            echo "WARNING: no presenteragent code found."
            read -p "Do you want to download from internet?(y/n, default:y)" confirm
            if [[ ${confirm}"X" != "X" && ${confirm} != "y" && ${confirm} != "Y" ]];then
                echo "ERROR: no presenteragent code found and no download choice, please put presenteragent code in ${script_path}/presenteragent path manually."
                return 1
            fi
        fi
    fi
    echo "Download presenteragent code..."
    presenteragent_download_url="https://github.com/Ascend/sdk-presenter/releases/download/${presenteragent_version}/presenteragent-${presenteragent_version}.zip"
    wget -O ${script_path}/presenteragent-${presenteragent_version}.ing ${presenteragent_download_url} --no-check-certificate
    if [[ $? -ne 0 ]];then
        echo "ERROR: download failed, please check ${presenteragent_download_url} connection."
        return 1
    fi

    mv ${script_path}/presenteragent-${presenteragent_version}.ing ${script_path}/presenteragent-${presenteragent_version}.zip
    unzip ${script_path}/presenteragent-${presenteragent_version}.zip -d ${script_path} 1>/dev/null
    if [[ $? -ne 0 ]];then
        echo "ERROR: uncompress presenteragent tar.gz file failed, please check ${presenteragent_download_url} connection."
        return 1
    fi
    mv ${script_path}/presenteragent-${presenteragent_version} ${script_path}/presenteragent
    rm -rf ${script_path}/presenteragent-${presenteragent_version}.zip
    rm -rf ${script_path}/presenteragent-${presenteragent_version}.ing
    return 0

}

function build_presenteragent()
{
    echo "Build presenteragent..."
    atlas_target=`grep "TARGET" ${DDK_HOME}/ddk_info | awk -F '"' '{print $4}'`
    if [[ $? -ne 0 ]];then
        echo "ERROR: can not get TARGET from ${DDK_HOME}/ddk_info, please check your env"
        return 1
    fi

    atlas_target=`echo ${atlas_target} | sed 's/ //g' `
     make clean mode=${atlas_target} -C ${script_path}/presenteragent 1>/dev/null
    if [[ $? -ne 0 ]];then
        echo "ERROR: compile presenteragent failed, please check the env."
        return 1
    fi
    make install mode=${atlas_target} -C ${script_path}/presenteragent 1>/dev/null
    if [[ $? -ne 0 ]];then
        echo "ERROR: compile presenteragent failed, please check the env."
        return 1
    fi
}

main()
{
    #download code
    download_code
    if [[ $? -ne 0 ]];then
        return 1
    fi

    build_presenteragent

    if [[ $? -ne 0 ]];then
        return 1
    fi

    echo "Finish to Build presenteragent."
    exit 0
}

main
