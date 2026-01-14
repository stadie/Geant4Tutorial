# syntax=docker/dockerfile:1

FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /build

RUN apt-get update && apt-get install -y \
    dpkg-dev cmake g++ gcc binutils libx11-dev libxpm-dev \
    libxft-dev libxext-dev python3 libssl-dev wget git \
    libtbb-dev libgif-dev python3-dev \
    libxerces-c-dev libgl1-mesa-dev libglu1-mesa-dev libxmu-dev libxi-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --branch v6-28-04 --depth=1 https://github.com/root-project/root.git \
    && mkdir root_build && cd root_build \
    && cmake -DCMAKE_INSTALL_PREFIX=/opt/root \
             -Dgdml=ON -Dmathmore=ON -Dbuiltin_vdt=ON ../root \
    && make -j$(nproc) && make install

# it didnt recognise the path automatically, so I added these lines
ENV ROOTSYS=/opt/root
ENV PATH=$ROOTSYS/bin:$PATH
ENV LD_LIBRARY_PATH=$ROOTSYS/lib
ENV CMAKE_PREFIX_PATH=$ROOTSYS


RUN git clone --depth=1 https://github.com/vmc-project/vmc.git \
    && mkdir vmc_core_build && cd vmc_core_build \
    && cmake -DCMAKE_INSTALL_PREFIX=/opt/root_vmc \
             -DROOT_DIR=/opt/root/cmake \
             -DCMAKE_PREFIX_PATH=/opt/root ../vmc \
    && make -j$(nproc) && make install

# same reason
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/root_vmc/lib
ENV CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/opt/root_vmc

# should be serial mode. without multi threading
# [CRITICAL FIX] We disable MT to ensure 100% compatibility with legacy code
RUN git clone --branch v11.1.2 --depth=1 https://github.com/Geant4/geant4.git \
    && mkdir g4_build && cd g4_build \
    && cmake -DCMAKE_INSTALL_PREFIX=/opt/geant4 \
             -DGEANT4_BUILD_MULTITHREADED=OFF \
             -DGEANT4_INSTALL_DATA=ON \
             -DGEANT4_USE_OPENGL_X11=ON \
             -DGEANT4_USE_GDML=ON \
             -DGEANT4_USE_QT=OFF ../geant4 \
    && make -j$(nproc) && make install


ENV Geant4_DIR=/opt/geant4/lib/cmake/Geant4
RUN git clone --branch v6-1 --depth=1 https://github.com/vmc-project/geant4_vmc.git \
    && mkdir vmc_build && cd vmc_build \
    && cmake -DCMAKE_INSTALL_PREFIX=/opt/geant4_vmc \
             -DGeant4VMC_USE_GEANT4_UI=ON \
             -DGeant4VMC_USE_GEANT4_VIS=ON ../geant4_vmc \
    && make -j$(nproc) && make install


##RUNTIME
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ gcc binutils cmake make \
    libx11-6 libxpm4 libxft2 libxext6 libssl3 python3 \
    libtbb12 libgif7 libxerces-c3.2 \
    libgl1 libglu1-mesa libxmu6 libxi6 \
    locales git emacs nano \
    && rm -rf /var/lib/apt/lists/*

#fix local issue
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8


COPY --from=builder /opt/root /opt/root
COPY --from=builder /opt/root_vmc /opt/root_vmc
COPY --from=builder /opt/geant4 /opt/geant4
COPY --from=builder /opt/geant4_vmc /opt/geant4_vmc


ENV ROOTSYS=/opt/root
ENV PATH=$ROOTSYS/bin:$PATH
ENV LD_LIBRARY_PATH=$ROOTSYS/lib:/opt/root_vmc/lib:/opt/geant4/lib:/opt/geant4_vmc/lib

# fix file not found errors
ENV ROOT_INCLUDE_PATH=/opt/root_vmc/include/vmc:/opt/root_vmc/include:/opt/geant4_vmc/include:/opt/geant4/include/Geant4

# fix TLS memory crash
ENV LD_PRELOAD="/opt/geant4/lib/libG4global.so /opt/geant4/lib/libG4materials.so /opt/geant4/lib/libG4geometry.so /opt/geant4/lib/libG4particles.so /opt/geant4/lib/libG4track.so /opt/geant4/lib/libG4processes.so"


RUN echo '#!/bin/bash' > /etc/profile.d/geant4_env.sh && \
    echo 'source /opt/geant4/bin/geant4.sh' >> /etc/profile.d/geant4_env.sh && \
    chmod +x /etc/profile.d/geant4_env.sh

RUN useradd -ms /bin/bash student
USER student
WORKDIR /home/student

ENTRYPOINT ["/bin/bash", "-c", "source /etc/profile.d/geant4_env.sh && exec \"$@\"", "--"]
CMD ["/bin/bash"]