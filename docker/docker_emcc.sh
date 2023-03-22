# export SKIP_PORTS=yes && ./docker_emcc.sh

PROJECT_PATH=$(pwd)/..
DOCKER_TYPE=emcc
DOCKER_IMAGE=onsyuri_${DOCKER_TYPE}
DOCKER_RUN=${DOCKER_IMAGE}_run
DOKCER_FILE=Dockerfile_${DOCKER_TYPE}
EMSDK_HOME=/opt/sdk/emsdk

if [ -z "$(docker images | grep ${DOCKER_IMAGE})" ]; then 
    docker build -f ${DOKCER_FILE} -t ${DOCKER_IMAGE} \
        --build-arg EMSDK_HOME=${EMSDK_HOME} ./
fi

if [ -z "$(docker ps | grep ${DOCKER_RUN})" ]; then
    docker run -d -it --rm -v ${PROJECT_PATH}:/project \
        --name ${DOCKER_RUN} ${DOCKER_IMAGE} bash
fi

docker exec -i -e TARGETS=$@ -e BUILD_TYPE=$BUILD_TYPE \
     ${DOCKER_RUN} bash -c \
     "cd /project/script && export EMSDK=$EMSDK_HOME &&bash cross_web.sh"