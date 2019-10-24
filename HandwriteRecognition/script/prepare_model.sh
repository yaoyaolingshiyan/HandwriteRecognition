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

tools_version=$1

app_path="${script_path}/.."

#function download_prepare_model_script()
#{
    #echo "download download_model.sh..."
    #rm -rf ${script_path}/download_model.sh
    
    #wget -O ${script_path}/download_model.sh "https://raw.githubusercontent.com/Ascend/models/master/download_model.sh" --no-check-certificate --quiet
    #if [ $? -ne 0 ];then
        #echo "ERROR: download failed, please check https://raw.githubusercontent.com/Ascend/models/master/download_modell.sh connetction."
        #rm -rf ${script_path}/download_model.sh
        #return 1
    #fi
    #return 0
#}

function prepare()
{
    model_names=$1

    if [[ ${tools_version}"X" == "X" ]];then
        echo "ERROR: Invalid tools version. please get tools_version from IDE help menu."
        return 1
    fi

    if [[ ${tools_version} == "local" ]];then
        for model_info in ${model_names}
        do
            model_name=`basename ${model_info}`
            if [ ! -f "${script_path}/${model_name}.om" ];then
                echo "ERROR: No ${model_name}.om in current path: ${script_path} for local mode."
                return 1
            else
                mkdir -p ${app_path}/MyModel/${model_name}/device
                cp ${script_path}/${model_name}.om ${app_path}/MyModel/${model_name}/device/
            fi
        done
    else
        for model_info in ${model_names}
        do
            model_name=`basename ${model_info}`
            if [ ! -f "${script_path}/${model_name}.om" ];then
                echo "ERROR: No ${model_name}.om in current path: ${script_path} for internet mode."
                return 1
            else
                mkdir -p ${app_path}/MyModel/${model_name}/device
                cp ${script_path}/${model_name}.om ${app_path}/MyModel/${model_name}/device/
            fi
        done
        #download_prepare_model_script
        #if [ $? -ne 0 ];then
            #return 1
        #fi
        #bash ${script_path}/download_model.sh ${tools_version} ${model_names}
        
        #if [ $? -ne 0 ];then
            #return 1
        #fi

        #for model_info in ${model_names}
        #do
            #model_name=`basename ${model_info}`
            #mkdir -p ${app_path}/MyModel/${model_name}/device
            #cp ${script_path}/${model_name}_${tools_version}.om ${app_path}/MyModel/${model_name}/device/${model_name}.om
        #done

    fi

    return 0
}

main()
{
    model_names="computer_vision/object_detect/hand_write"
    prepare ${model_names}
    if [ $? -ne 0 ];then
        exit 1
    fi
    exit 0
}

main
