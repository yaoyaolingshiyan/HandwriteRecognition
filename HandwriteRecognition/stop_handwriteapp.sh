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

. ${script_path}/script/func_util.sh
. ${script_path}/script/func_deploy.sh

function kill_remote_running()
{
    echo -e "run.sh exit, kill ${remote_host}:ascend_handwriteapp running..."
    parse_remote_port
	iRet=$(IDE-daemon-client --host ${remote_host}:${remote_port} --hostcmd "kill \$(pidof ~/HIAI_PROJECTS/ascend_workspace/handwriteapp/out/ascend_handwriteapp)")
	if [[ $? -eq 0 ]];then
		echo "$iRet in ${remote_host}."
		return 0
	fi

    iRet=$(IDE-daemon-client --host ${remote_host}:${remote_port} --hostcmd "for p in \`pidof ascend_handwriteapp\`; do { echo \"kill \$p\"; kill \$p; }; done")
    if [[ $? -ne 0 ]];then
        echo "ERROR: kill ${remote_host}:ascend_handwriteapp running failed, please login to kill it manually."
        return 1
    else
        echo "$iRet in ${remote_host}."
    fi
    return 0
}

function main()
{
    check_ip_addr ${remote_host}
    if [[ $? -ne 0 ]];then
        echo "ERROR: invalid host ip, please check your command format: ./stop_handwriteapp.sh host_ip."
        exit 1
    fi

    running_pid=`ps -ef | grep "run_handwriteapp\.sh" | grep "${remote_host}" | awk -F ' ' '{print $2}'`

    if [[ ${running_pid}"X" != "X" ]];then
        echo "kill local runn_face run_handwriteapp.sh ${running_pid}"
        kill -9 ${running_pid}
    fi	
	
    parse_remote_port

    kill_remote_running
    if [[ $? -ne 0 ]];then
        exit 1
    fi
    echo "kill presenter server "
    kill -9 $(ps -ef | grep "presenter" | grep "hand_write" | awk -F ' ' '{print $2}')
    exit 0
}

main
