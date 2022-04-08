FROM alpine:3.12

LABEL maintainer="Roger Light <roger@atchoo.org>" \
    description="Eclipse Mosquitto MQTT Broker"

ENV VERSION=2.0.9 \
    DOWNLOAD_SHA256=1b8553ef64a1cf5e4f4cfbe098330ae612adccd3d37f35b2db6f6fab501b01d4 \
    GPG_KEYS=A0D6EEA1DCAE49A635A3B2F0779B22DFB3E717B7 \
    LWS_VERSION=2.4.2 \
    LWS_SHA256=73012d7fcf428dedccc816e83a63a01462e27819d5537b8e0d0c7264bfacfad6 \
    CJSON_VERSION=1.7.14 \
    CJSON_SHA256=fb50a663eefdc76bafa80c82bc045af13b1363e8f45cec8b442007aef6a41343

RUN set -x && \
    apk --no-cache add --virtual build-deps \
        build-base \
        cmake \
        gnupg \
        openssl-dev \
        util-linux-dev && \
    wget https://github.com/warmcat/libwebsockets/archive/v${LWS_VERSION}.tar.gz -O /tmp/lws.tar.gz && \
    echo "$LWS_SHA256  /tmp/lws.tar.gz" | sha256sum -c - && \
    mkdir -p /build/lws && \
    tar --strip=1 -xf /tmp/lws.tar.gz -C /build/lws && \
    rm /tmp/lws.tar.gz && \
    cd /build/lws && \
    cmake . \
        -DCMAKE_BUILD_TYPE=MinSizeRel \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DLWS_IPV6=ON \
        -DLWS_WITHOUT_BUILTIN_GETIFADDRS=ON \
        -DLWS_WITHOUT_CLIENT=ON \
        -DLWS_WITHOUT_EXTENSIONS=ON \
        -DLWS_WITHOUT_TESTAPPS=ON \
        -DLWS_WITH_SHARED=OFF \
        -DLWS_WITH_ZIP_FOPS=OFF \
        -DLWS_WITH_ZLIB=OFF && \
    make -j "$(nproc)" && \
    rm -rf /root/.cmake && \
    wget https://github.com/DaveGamble/cJSON/archive/v${CJSON_VERSION}.tar.gz -O /tmp/cjson.tar.gz && \
    echo "$CJSON_SHA256  /tmp/cjson.tar.gz" | sha256sum -c - && \
    mkdir -p /build/cjson && \
    tar --strip=1 -xf /tmp/cjson.tar.gz -C /build/cjson && \
    rm /tmp/cjson.tar.gz && \
    cd /build/cjson && \
    cmake . \
        -DCMAKE_BUILD_TYPE=MinSizeRel \
        -DBUILD_SHARED_AND_STATIC_LIBS=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        -DCJSON_BUILD_SHARED_LIBS=OFF \
        -DCJSON_OVERRIDE_BUILD_SHARED_LIBS=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr && \
    make -j "$(nproc)" && \
    rm -rf /root/.cmake && \
    wget https://mosquitto.org/files/source/mosquitto-${VERSION}.tar.gz -O /tmp/mosq.tar.gz && \
    echo "$DOWNLOAD_SHA256  /tmp/mosq.tar.gz" | sha256sum -c - && \
    wget https://mosquitto.org/files/source/mosquitto-${VERSION}.tar.gz.asc -O /tmp/mosq.tar.gz.asc && \
    export GNUPGHOME="$(mktemp -d)" && \
    found=''; \
    for server in \
        ha.pool.sks-keyservers.net \
        hkp://keyserver.ubuntu.com:80 \
        hkp://p80.pool.sks-keyservers.net:80 \
        pgp.mit.edu \
    ; do \
        echo "Fetching GPG key $GPG_KEYS from $server"; \
        gpg --keyserver "$server" --keyserver-options timeout=10 --recv-keys "$GPG_KEYS" && found=yes && break; \
    done; \
    test -z "$found" && echo >&2 "error: failed to fetch GPG key $GPG_KEYS" && exit 1; \
    gpg --batch --verify /tmp/mosq.tar.gz.asc /tmp/mosq.tar.gz && \
    gpgconf --kill all && \
    rm -rf "$GNUPGHOME" /tmp/mosq.tar.gz.asc && \
    mkdir -p /build/mosq && \
    tar --strip=1 -xf /tmp/mosq.tar.gz -C /build/mosq && \
    rm /tmp/mosq.tar.gz && \
    make -C /build/mosq -j "$(nproc)" \
        CFLAGS="-Wall -O2 -I/build/lws/include -I/build" \
        LDFLAGS="-L/build/lws/lib -L/build/cjson" \
        WITH_ADNS=no \
        WITH_DOCS=no \
        WITH_SHARED_LIBRARIES=yes \
        WITH_SRV=no \
        WITH_STRIP=yes \
        WITH_WEBSOCKETS=yes \
        prefix=/usr \
        binary && \
    addgroup -S -g 1883 mosquitto 2>/dev/null && \
    adduser -S -u 1883 -D -H -h /var/empty -s /sbin/nologin -G mosquitto -g mosquitto mosquitto 2>/dev/null && \
    mkdir -p /mosquitto/config /mosquitto/data /mosquitto/log && \
    install -d /usr/sbin/ && \
    install -s -m755 /build/mosq/client/mosquitto_pub /usr/bin/mosquitto_pub && \
    install -s -m755 /build/mosq/client/mosquitto_rr /usr/bin/mosquitto_rr && \
    install -s -m755 /build/mosq/client/mosquitto_sub /usr/bin/mosquitto_sub && \
    install -s -m644 /build/mosq/lib/libmosquitto.so.1 /usr/lib/libmosquitto.so.1 && \
    install -s -m755 /build/mosq/src/mosquitto /usr/sbin/mosquitto && \
    install -s -m755 /build/mosq/apps/mosquitto_ctrl/mosquitto_ctrl /usr/bin/mosquitto_ctrl && \
    install -s -m755 /build/mosq/apps/mosquitto_passwd/mosquitto_passwd /usr/bin/mosquitto_passwd && \
    install -s -m755 /build/mosq/plugins/dynamic-security/mosquitto_dynamic_security.so /usr/lib/mosquitto_dynamic_security.so && \
    install -m644 /build/mosq/mosquitto.conf /mosquitto/config/mosquitto.conf && \
    chown -R mosquitto:mosquitto /mosquitto && \
    apk --no-cache add \
        ca-certificates && \
    apk del build-deps && \
    rm -rf /build

VOLUME ["/mosquitto/data", "/mosquitto/log"]

# Set up the entry point script and default command
COPY docker-entrypoint.sh mosquitto-no-auth.conf /
EXPOSE 1883
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]
