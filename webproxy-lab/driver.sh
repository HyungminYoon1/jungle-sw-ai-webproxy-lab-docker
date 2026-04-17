#!/bin/bash
#
# driver.sh - Proxy Lab용 간단한 자동 채점 스크립트입니다.
#     코드가 동시성 캐시 프록시처럼 동작하는지 확인하는
#     기본적인 정상 동작 검사를 수행합니다.
#
#     David O'Hallaron, Carnegie Mellon University
#     수정일: 2016-02-08
# 
#     사용법: ./driver.sh
# 

# 점수 배점
MAX_BASIC=40
MAX_CONCURRENCY=15
MAX_CACHE=15

# 각종 상수
HOME_DIR=`pwd`
PROXY_DIR="./.proxy"
NOPROXY_DIR="./.noproxy"
TIMEOUT=5
MAX_RAND=63000
PORT_START=1024
PORT_MAX=65000
MAX_PORT_TRIES=10

# 기본 테스트에 사용할 텍스트 및 바이너리 파일 목록
BASIC_LIST="home.html
            csapp.c
            tiny.c
            godzilla.jpg
            tiny"

# 캐시 테스트에 사용할 텍스트 파일 목록
CACHE_LIST="tiny.c
            home.html
            csapp.c"

# 여러 테스트에서 가져올 파일
FETCH_FILE="home.html"

#####
# 보조 함수
#

#
# download_proxy - 프록시를 거쳐 원 서버에서 파일을 내려받습니다
# 사용법: download_proxy <testdir> <filename> <origin_url> <proxy_url>
#
function download_proxy {
    cd $1
    curl --max-time ${TIMEOUT} --silent --proxy $4 --output $2 $3
    (( $? == 28 )) && echo "Error: Fetch timed out after ${TIMEOUT} seconds"
    cd $HOME_DIR
}

#
# download_noproxy - 원 서버에서 파일을 직접 내려받습니다
# 사용법: download_noproxy <testdir> <filename> <origin_url>
#
function download_noproxy {
    cd $1
    curl --max-time ${TIMEOUT} --silent --output $2 $3 
    (( $? == 28 )) && echo "Error: Fetch timed out after ${TIMEOUT} seconds"
    cd $HOME_DIR
}

#
# clear_dirs - 다운로드 디렉터리를 비웁니다
#
function clear_dirs {
    rm -rf ${PROXY_DIR}/*
    rm -rf ${NOPROXY_DIR}/*
}

#
# wait_for_port_use - 인자로 받은 TCP 포트가 실제로 사용될 때까지
#     반복해서 확인합니다. 5초 후 타임아웃됩니다.
#
function wait_for_port_use() {
    timeout_count="0"
    portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
        | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
        | grep -E "[0-9]+" | uniq | tr "\n" " "`

    echo "${portsinuse}" | grep -wq "${1}"
    while [ "$?" != "0" ]
    do
        timeout_count=`expr ${timeout_count} + 1`
        if [ "${timeout_count}" == "${MAX_PORT_TRIES}" ]; then
            kill -ALRM $$
        fi

        sleep 1
        portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
            | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
            | grep -E "[0-9]+" | uniq | tr "\n" " "`
        echo "${portsinuse}" | grep -wq "${1}"
    done
}


#
# free_port - 사용 가능한 빈 TCP 포트를 반환합니다 
#
function free_port {
    # [PORT_START, PORT_START+MAX_RAND] 범위에서 무작위 포트를 생성합니다.
    # 여러 학생이 같은 머신에서 driver를 실행할 때
    # 충돌을 피하기 위해 필요합니다.
    port=$((( RANDOM % ${MAX_RAND}) + ${PORT_START}))

    while [ TRUE ] 
    do
        portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
            | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
            | grep -E "[0-9]+" | uniq | tr "\n" " "`

        echo "${portsinuse}" | grep -wq "${port}"
        if [ "$?" == "0" ]; then
            if [ $port -eq ${PORT_MAX} ]
            then
                echo "-1"
                return
            fi
            port=`expr ${port} + 1`
        else
            echo "${port}"
            return
        fi
    done
}


#######
# 메인 
#######

######
# 예상한 파일이 모두 있고 권한도 올바른지 확인
#

# 현재 사용자가 소유한 남아 있는 proxy 또는 tiny 서버를 종료
killall -q proxy tiny nop-server.py 2> /dev/null

# Tiny 디렉터리가 있는지 확인
if [ ! -d ./tiny ]
then 
    echo "Error: ./tiny directory not found."
    exit
fi

# Tiny 실행 파일이 없으면 빌드를 시도
if [ ! -x ./tiny/tiny ]
then 
    echo "Building the tiny executable."
    (cd ./tiny; make)
    echo ""
fi

# 필요한 Tiny 파일이 모두 있는지 확인
if [ ! -x ./tiny/tiny ]
then 
    echo "Error: ./tiny/tiny not found or not an executable file."
    exit
fi
for file in ${BASIC_LIST}
do
    if [ ! -e ./tiny/${file} ]
    then
        echo "Error: ./tiny/${file} not found."
        exit
    fi
done

# 실행 가능한 proxy 파일이 있는지 확인
if [ ! -x ./proxy ]
then 
    echo "Error: ./proxy not found or not an executable file. Please rebuild your proxy and try again."
    exit
fi

# 실행 가능한 nop-server.py 파일이 있는지 확인
if [ ! -x ./nop-server.py ]
then 
    echo "Error: ./nop-server.py not found or not an executable file."
    exit
fi

# 필요하면 테스트 디렉터리를 생성
if [ ! -d ${PROXY_DIR} ]
then
    mkdir ${PROXY_DIR}
fi

if [ ! -d ${NOPROXY_DIR} ]
then
    mkdir ${NOPROXY_DIR}
fi

# 의미 있는 타임아웃 메시지를 출력할 핸들러 추가
trap 'echo "Timeout waiting for the server to grab the port reserved for it"; kill $$' ALRM

#####
# 기본
#
echo "*** 기본 ***"

# Tiny 웹 서버 실행
tiny_port=$(free_port)
echo "Starting tiny on ${tiny_port}"
cd ./tiny
./tiny ${tiny_port}   &> /dev/null  &
tiny_pid=$!
cd ${HOME_DIR}

# tiny가 실제로 시작될 때까지 대기
wait_for_port_use "${tiny_port}"

# proxy 실행
proxy_port=$(free_port)
echo "Starting proxy on ${proxy_port}"
./proxy ${proxy_port}  &> /dev/null &
proxy_pid=$!

# proxy가 실제로 시작될 때까지 대기
wait_for_port_use "${proxy_port}"


# 이제 Tiny에서 직접, 그리고 proxy를 통해 텍스트 및 바이너리 파일을
# 가져온 뒤 결과를 비교해 테스트를 수행합니다.
numRun=0
numSucceeded=0
for file in ${BASIC_LIST}
do
    numRun=`expr $numRun + 1`
    echo "${numRun}: ${file}"
    clear_dirs

    # proxy를 통해 가져오기
    echo "   Fetching ./tiny/${file} into ${PROXY_DIR} using the proxy"
    download_proxy $PROXY_DIR ${file} "http://localhost:${tiny_port}/${file}" "http://localhost:${proxy_port}"

    # Tiny에서 직접 가져오기
    echo "   Fetching ./tiny/${file} into ${NOPROXY_DIR} directly from Tiny"
    download_noproxy $NOPROXY_DIR ${file} "http://localhost:${tiny_port}/${file}"

    # 두 파일 비교
    echo "   Comparing the two files"
    diff -q ${PROXY_DIR}/${file} ${NOPROXY_DIR}/${file} &> /dev/null
    if [ $? -eq 0 ]; then
        numSucceeded=`expr ${numSucceeded} + 1`
        echo "   Success: Files are identical."
    else
        echo "   Failure: Files differ."
    fi
done

echo "Killing tiny and proxy"
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null

basicScore=`expr ${MAX_BASIC} \* ${numSucceeded} / ${numRun}`

echo "basicScore: $basicScore/${MAX_BASIC}"


######
# 동시성
#

echo ""
echo "*** 동시성 ***"

# Tiny 웹 서버 실행
tiny_port=$(free_port)
echo "Starting tiny on port ${tiny_port}"
cd ./tiny
./tiny ${tiny_port} &> /dev/null &
tiny_pid=$!
cd ${HOME_DIR}

# tiny가 실제로 시작될 때까지 대기
wait_for_port_use "${tiny_port}"

# proxy 실행
proxy_port=$(free_port)
echo "Starting proxy on port ${proxy_port}"
./proxy ${proxy_port} &> /dev/null &
proxy_pid=$!

# proxy가 실제로 시작될 때까지 대기
wait_for_port_use "${proxy_port}"

# 요청에 응답하지 않는 특별한 blocking nop-server 실행
nop_port=$(free_port)
echo "Starting the blocking NOP server on port ${nop_port}"
./nop-server.py ${nop_port} &> /dev/null &
nop_pid=$!

# nop 서버가 실제로 시작될 때까지 대기
wait_for_port_use "${nop_port}"

# proxy를 통해 blocking nop-server에서 파일을 가져오도록 시도
clear_dirs
echo "Trying to fetch a file from the blocking nop-server"
download_proxy $PROXY_DIR "nop-file.txt" "http://localhost:${nop_port}/nop-file.txt" "http://localhost:${proxy_port}" &

# Tiny에서 직접 가져오기
echo "Fetching ./tiny/${FETCH_FILE} into ${NOPROXY_DIR} directly from Tiny"
download_noproxy $NOPROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}"

# proxy를 통해 가져오기
echo "Fetching ./tiny/${FETCH_FILE} into ${PROXY_DIR} using the proxy"
download_proxy $PROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}" "http://localhost:${proxy_port}"

# proxy를 통한 가져오기가 성공했는지 확인
echo "Checking whether the proxy fetch succeeded"
diff -q ${PROXY_DIR}/${FETCH_FILE} ${NOPROXY_DIR}/${FETCH_FILE} &> /dev/null
if [ $? -eq 0 ]; then
    concurrencyScore=${MAX_CONCURRENCY}
    echo "Success: Was able to fetch tiny/${FETCH_FILE} from the proxy."
else
    concurrencyScore=0
    echo "Failure: Was not able to fetch tiny/${FETCH_FILE} from the proxy."
fi

# 정리
echo "Killing tiny, proxy, and nop-server"
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null
kill $nop_pid 2> /dev/null
wait $nop_pid 2> /dev/null

echo "concurrencyScore: $concurrencyScore/${MAX_CONCURRENCY}"

#####
# 캐시
#
echo ""
echo "*** Cache ***"

# Tiny 웹 서버 실행
tiny_port=$(free_port)
echo "Starting tiny on port ${tiny_port}"
cd ./tiny
./tiny ${tiny_port} &> /dev/null &
tiny_pid=$!
cd ${HOME_DIR}

# tiny가 실제로 시작될 때까지 대기
wait_for_port_use "${tiny_port}"

# proxy 실행
proxy_port=$(free_port)
echo "Starting proxy on port ${proxy_port}"
./proxy ${proxy_port} &> /dev/null &
proxy_pid=$!

# proxy가 실제로 시작될 때까지 대기
wait_for_port_use "${proxy_port}"

# proxy를 사용해 tiny에서 몇 개의 파일 가져오기
clear_dirs
for file in ${CACHE_LIST}
do
    echo "Fetching ./tiny/${file} into ${PROXY_DIR} using the proxy"
    download_proxy $PROXY_DIR ${file} "http://localhost:${tiny_port}/${file}" "http://localhost:${proxy_port}"
done

# Tiny 종료
echo "Killing tiny"
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null

# 이제 가져왔던 파일 중 하나의 캐시된 복사본을 가져와 봅니다.
echo "Fetching a cached copy of ./tiny/${FETCH_FILE} into ${NOPROXY_DIR}"
download_proxy $NOPROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}" "http://localhost:${proxy_port}"

# tiny 디렉터리의 원본 파일과 비교하여
# proxy를 통한 가져오기가 성공했는지 확인
diff -q ./tiny/${FETCH_FILE} ${NOPROXY_DIR}/${FETCH_FILE}  &> /dev/null
if [ $? -eq 0 ]; then
    cacheScore=${MAX_CACHE}
    echo "Success: Was able to fetch tiny/${FETCH_FILE} from the cache."
else
    cacheScore=0
    echo "Failure: Was not able to fetch tiny/${FETCH_FILE} from the proxy cache."
fi

# proxy 종료
echo "Killing proxy"
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null

echo "cacheScore: $cacheScore/${MAX_CACHE}"

# 총점을 출력
totalScore=`expr ${basicScore} + ${cacheScore} + ${concurrencyScore}`
maxScore=`expr ${MAX_BASIC} + ${MAX_CACHE} + ${MAX_CONCURRENCY}`
echo ""
echo "totalScore: ${totalScore}/${maxScore}"
exit

