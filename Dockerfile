FROM condaforge/miniforge3:24.9.2-0

SHELL ["/bin/bash", "-c"]
ARG APPS_DIR=/usr/local
RUN conda create -y --name geant --channel=conda-forge geant4 root cmake make
RUN conda clean --all
RUN conda init

ARG VMC_DIR=$APPS_DIR/vmc
RUN . ~/.bashrc
    conda activate geant && \
    git clone http://github.com/vmc-project/vmc.git $VMC_DIR/git_source && \
    cd $VMC_DIR && \
    git checkout v2-0 && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=$VMC_DIR $VMC_DIR/git_source && \
    make -j4 && \
    maken install && \
    rm -rf build git_source


