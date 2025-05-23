#  Copyright (c) 2018, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

ARG BASE_IMAGE=ubuntu:bionic
FROM ${BASE_IMAGE}

ARG INFRA_IF_NAME
ARG BORDER_ROUTING
ARG BACKBONE_ROUTER
ARG OT_BACKBONE_CI
ARG OTBR_OPTIONS
ARG NAT64
ARG NAT64_DYNAMIC_POOL
ARG REFERENCE_DEVICE
ARG RELEASE
ARG REST_API
ARG WEB_GUI
ARG MDNS
ARG FIREWALL
ARG OT_SRP_ADV_PROXY

ENV INFRA_IF_NAME=${INFRA_IF_NAME:-eth0}
ENV BORDER_ROUTING=${BORDER_ROUTING:-1}
ENV BACKBONE_ROUTER=${BACKBONE_ROUTER:-1}
ENV OT_BACKBONE_CI=${OT_BACKBONE_CI:-0}
ENV OTBR_MDNS=${MDNS:-mDNSResponder}
ENV OTBR_OPTIONS=${OTBR_OPTIONS}
ENV DEBIAN_FRONTEND noninteractive
ENV PLATFORM ubuntu
ENV REFERENCE_DEVICE=${REFERENCE_DEVICE:-0}
ENV RELEASE=${RELEASE:-1}
ENV NAT64=${NAT64:-1}
ENV NAT64_DYNAMIC_POOL=${NAT64_DYNAMIC_POOL:-192.168.255.0/24}
ENV WEB_GUI=${WEB_GUI:-1}
ENV REST_API=${REST_API:-1}
ENV FIREWALL=${FIREWALL:-1}
ENV OT_SRP_ADV_PROXY=${OT_SRP_ADV_PROXY:-0}
ENV DOCKER 1

RUN env

# Required during build or run
ENV OTBR_DOCKER_REQS sudo python3

# Required during build, could be removed
ENV OTBR_DOCKER_DEPS git ca-certificates

# Required and installed during build (script/bootstrap), could be removed
ENV OTBR_BUILD_DEPS apt-utils build-essential psmisc ninja-build cmake wget ca-certificates \
  libreadline-dev libncurses-dev libdbus-1-dev libavahi-common-dev \
  libavahi-client-dev \
  libnetfilter-queue-dev

# Required for OpenThread Backbone CI
ENV OTBR_OT_BACKBONE_CI_DEPS curl lcov wget build-essential python3-dbus python3-zeroconf socat

# Resolves issue with installing libc-bin
RUN rm /var/lib/dpkg/info/libc-bin.* \
  && apt-get clean -y \
  && apt-get update -y \
  && apt-get install --no-install-recommends -y libc-bin

RUN apt-get update \
  && apt-get install --no-install-recommends -y $OTBR_DOCKER_REQS $OTBR_DOCKER_DEPS \
  && ([ "${OT_BACKBONE_CI}" != "1" ] || apt-get install --no-install-recommends -y $OTBR_OT_BACKBONE_CI_DEPS) \
  && ln -fs /usr/share/zoneinfo/UTC /etc/localtime

COPY ./script /app/script
COPY ./third_party/mDNSResponder /app/third_party/mDNSResponder
WORKDIR /app

RUN ./script/bootstrap
COPY . .
RUN ./script/setup

RUN ([ "${OT_BACKBONE_CI}" = "1" ] || ( \
    mv ./script /tmp \
    && mv ./etc /tmp \
    && find . -delete \
    && rm -rf /usr/include \
    && mv /tmp/script . \
    && mv /tmp/etc . \
    && apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false $OTBR_DOCKER_DEPS \
    && apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false $OTBR_BUILD_DEPS  \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /tmp/* \
  ))

ENTRYPOINT ["/app/etc/docker/test/docker_entrypoint.sh"]

EXPOSE 80
