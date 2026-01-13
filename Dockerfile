FROM condaforge/miniforge3:25.11.0-0

#ARG APPS_DIR=/usr/local
RUN useradd -ms /bin/bash student
USER student
WORKDIR /home/student
ARG APPS_DIR=/home/student
RUN conda create -y --name geant --channel=conda-forge geant4=11.3 root=6.32 cmake make
RUN conda clean --all -y

SHELL ["conda", "run", "-n", "geant", "/bin/bash", "-c"]

ARG VMC_DIR=$APPS_DIR/vmc
RUN mkdir $VMC_DIR
RUN git clone --depth 1 --branch v2-1 http://github.com/vmc-project/vmc.git $VMC_DIR/git_source && \
    cd $VMC_DIR/git_source && \
    cd .. && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$VMC_DIR $VMC_DIR/git_source && \
    make -j4 && \
    make install && \
    cd .. && \
    rm -rf build git_source


ARG VGM_DIR=$APPS_DIR/vgm
RUN mkdir $VGM_DIR
RUN git clone --depth 1 --branch v5-4 http://github.com/vmc-project/vgm.git $VGM_DIR/git_source && \
    cd $VGM_DIR/git_source && \
    cd .. && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$VGM_DIR -DWITH_EXAMPLES=OFF -DINSTALL_EXAMPLES=OFF $VGM_DIR/git_source && \
    make -j4 && \
    make install && \
    cd .. && \
    rm -rf build git_source

ARG GEANT4_VMC_DIR=$APPS_DIR/geant4_vmc
RUN mkdir $GEANT4_VMC_DIR
RUN git clone --depth 1 --branch  v6-7 http://github.com/vmc-project/geant4_vmc.git $GEANT4_VMC_DIR/git_source && \
    cd $GEANT4_VMC_DIR/git_source && \
    cd .. && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$GEANT4_VMC_DIR -DGeant4VMC_USE_VGM=ON -DGeant4VMC_BUILD_EXAMPLES=ON -DGeant4VMC_USE_GEANT4_UI=OFF -DGeant4VMC_USE_GEANT4_VIS=OFF -DGeant4VMC_INSTALL_EXAMPLES=ON $GEANT4_VMC_DIR/git_source && \
    make -j4 && \
    make install && \
    cd .. && \
    rm -rf build git_source

ARG USE_VGM=1
RUN export LD_LIBRARY_PATH=$VGM_DIR/lib:VMC_DIR/lib:GEANT4_VMC_DIR/lib:$LD_LIBRARY_PATH && \
    echo $LD_LIBRARY_PATH
