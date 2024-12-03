FROM condaforge/miniforge3:24.9.2-0

ARG APPS_DIR=/usr/local
RUN conda create -y --name geant --channel=conda-forge geant4 root cmake make
RUN conda clean --all -y

SHELL ["conda", "run", "-n", "geant", "/bin/bash", "-c"]

ARG VMC_DIR=$APPS_DIR/vmc
RUN mkdir $VMC_DIR
RUN git clone http://github.com/vmc-project/vmc.git $VMC_DIR/git_source && \
    cd $VMC_DIR/git_source && \
    git checkout v2-0 && \
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
RUN git clone http://github.com/vmc-project/vgm.git $VGM_DIR/git_source && \
    cd $VGM_DIR/git_source && \
    git checkout v5-2 && \
    cd .. && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$VGM_DIR -DWITH_EXAMPLES=OFF -DINSTALL_EXAMPLES=OFF $VGM_DIR/git_source && \
    make -j4 && \
    make install && \
    cd .. && \
    rm -rf build git_source
