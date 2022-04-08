#!/bin/bash

set -e

FLAGS="-I. -O2 -ffreestanding -fno-builtin -nostdlib -ffunction-sections -fdata-sections -Werror"
LINK_OPT="build/libplatform.a -T link.ld"

mkdir -p build/o

${CC} -c platform/platform.c -o build/o/platform.o ${FLAGS}
${CC} -c platform/platform_asm.s -o build/o/platform_asm.o ${FLAGS}
${CC} -c platform/stdlib.c -o build/o/stdlib.o ${FLAGS}

${AR} r build/libplatform.a \
    build/o/platform.o \
    build/o/platform_asm.o \
    build/o/stdlib.o

# ${CC} echo/main.c -o build/echo.s -S ${FLAGS}
# ${CC} echo/main.c -o build/echo ${FLAGS} ${LINK_OPT}

PROJECTS=(
    hello-world
    hello-world-timer
    test-0-echo-d0
    test-0-echo-d1
)


for project in "${PROJECTS[@]}"
do
    ${CC} ${project}/main.c -o build/${project} ${FLAGS} ${LINK_OPT}
done