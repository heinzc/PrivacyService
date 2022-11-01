FROM qt6.4:latest as base

ENV INSTALL_PREFIX=/opt/privacyservice

RUN apt-get update
RUN apt-get install -qq -y --no-install-recommends \
      # Decompression
        xz-utils \
        unzip \
        bzip2 \
      # Building
        git \
		ninja-build \
        cmake \
		g++-${GCC_VERSION} \
	  # dependency of Qt
		#libmd4c-dev \
		#libmd4c-html0 \
    #libdouble-conversion3 \
    # dependency of SEAL
        libpcre2-16-0 \
      -
RUN \
    apt-get build-dep -y --no-install-recommends qtbase-opensource-src


FROM base as builder

ENV GIT_SSL_NO_VERIFY=true

WORKDIR /root/PrivacyService
ADD . ./

RUN \
    mkdir build_docker; \
    mkdir /opt/privacyservice; \
    cd build_docker; \
    cmake -G Ninja -DCMAKE_PREFIX_PATH=/usr/local ..; \
    cmake --build . --parallel; \
    cmake --install . --prefix $INSTALL_PREFIX

###############################################################################
# Resulting Docker Image ######################################################

FROM base as deploy

COPY --from=builder "${INSTALL_PREFIX}" "${INSTALL_PREFIX}"

RUN \
    mkdir $INSTALL_PREFIX/bin/data; \
    rm -rf /var/lib/apt/lists/*

WORKDIR $INSTALL_PREFIX/bin

EXPOSE 4242

USER root
ENV HOME $INSTALL_PREFIX

ENTRYPOINT $INSTALL_PREFIX"/bin/PrivacyService"
# CMD ["/bin/bash"]

#/opt/privacyservice/bin/PrivacyService