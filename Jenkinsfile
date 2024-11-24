pipeline {

    agent any

    environment {
        NAME = 'Flits'
        BASE_NAME = 'flits'
        ZIP_NAME = 'FLS'
    }

    stages {

        stage("depends") {

            steps {
                echo 'building depends ...'
                sh '''#!/bin/bash
                    cd depends
                    make -j $(nproc) HOST=x86_64-pc-linux-gnu
                    make -j $(nproc) HOST=x86_64-w64-mingw32
                    make -j $(nproc) HOST=aarch64-linux-gnu
                    make -j $(nproc) HOST=arm-linux-gnueabihf
                    rm -rf SDKs
                    mkdir SDKs
                    cd SDKs
                    wget -c https://ipfs.decenomy.net/QmccbpJcugkoV6uVovYsyZ3MPxGTcbUNYXMhLQL2hLzHwH -O MacOSX10.11.sdk.tar.xz
                    tar -xf MacOSX10.11.sdk.tar.xz
                    cd ..
                    make -j $(nproc) HOST=x86_64-apple-darwin14
                '''
            }
        }

        stage("build_x86_64-pc-linux-gnu") {

            steps {
                echo 'building linux ...'
                sh '''#!/bin/bash
                    make clean
                    ./autogen.sh
                    ./configure --enable-glibc-back-compat --prefix=$(pwd)/depends/x86_64-pc-linux-gnu LDFLAGS="-static-libstdc++" --enable-cxx --enable-static --disable-shared --disable-debug --disable-tests --disable-bench --with-pic --enable-upnp-default CPPFLAGS="-fPIC -O3" CXXFLAGS="-fPIC -O3"
	                make -j $(nproc) HOST=x86_64-pc-linux-gnu
                '''
            }
        }

        stage("deploy_x86_64-pc-linux-gnu") {

            steps {
                echo 'deploy linux ...'

                sh """#!/bin/bash
                    mkdir -p deploy/linux
                    strip -s src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt
                    cp src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt deploy/linux/
                    cd deploy/linux
                    touch ${BASE_NAME}-cli
                    rm ../${BASE_NAME}-cli
                    cp ${BASE_NAME}-cli ../
                    zip ${ZIP_NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-Linux.zip ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt
                    rm -f ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt
                """
            }
        }

        stage("build_x86_64-w64-mingw32") {

            steps {
                echo 'building windows ...'
                sh '''#!/bin/bash
                    make clean
                    ./autogen.sh
                    ./configure --prefix=$(pwd)/depends/x86_64-w64-mingw32 --disable-debug --disable-tests --disable-bench --enable-upnp-default CFLAGS="-O3" CXXFLAGS="-O3"
	                make -j $(nproc) HOST=x86_64-w64-mingw32
                '''
            }
        }

        stage("deploy_x86_64-w64-mingw32") {

            steps {
                echo 'deploy windows ...'
                sh """#!/bin/bash
                    mkdir -p deploy/windows
                    strip -s src/${BASE_NAME}d.exe src/${BASE_NAME}-cli.exe src/${BASE_NAME}-tx.exe src/qt/${BASE_NAME}-qt.exe
                    cp src/${BASE_NAME}d.exe src/${BASE_NAME}-cli.exe src/${BASE_NAME}-tx.exe src/qt/${BASE_NAME}-qt.exe deploy/windows/
                    cd deploy/windows
                    zip ${ZIP_NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-Windows.zip ${BASE_NAME}d.exe ${BASE_NAME}-cli.exe ${BASE_NAME}-tx.exe ${BASE_NAME}-qt.exe
                    mkdir -p ../../contrib/innosetup/package
                    mv ${BASE_NAME}* ../../contrib/innosetup/package/
                    cd ../../contrib/innosetup/
                    wine ~/.wine/drive_c/Program\\ Files\\ \\(x86\\)/Inno\\ Setup\\ 6/ISCC.exe setup.iss
                    mv output/${NAME}Setup.exe ../../deploy/windows/${NAME}-\$(../../deploy/${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-Setup.exe
                    cd ../../deploy/windows/
                    zip ${ZIP_NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-WindowsSetup.zip ${NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-Setup.exe
                    rm -f ${NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-Setup.exe 
                """
            }
        }

        stage("build_x86_64-apple-darwin14") {

            steps {
                echo 'building macos ...'
                sh '''#!/bin/bash
                    make clean
                    ./autogen.sh
                    ./configure --prefix=$(pwd)/depends/x86_64-apple-darwin14 --enable-cxx --enable-static --disable-shared --disable-debug --disable-tests --disable-bench --enable-upnp-default
                    make -j $(nproc) HOST=x86_64-apple-darwin14
                '''
            }
        }

        stage("deploy_x86_64-apple-darwin14") {

            steps {
                echo 'deploy macos ...'
                sh """#!/bin/bash
                    strip -s src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt
                    make deploy HOST=x86_64-apple-darwin14
                    mkdir -p deploy/macos
                    cp src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt ${NAME}-Core.dmg deploy/macos/
                    cd deploy/macos
                    zip ${ZIP_NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-MacOS.zip ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt ${NAME}-Core.dmg
                    rm -f ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt ${NAME}-Core.dmg
                """
            }
        }

        stage("build_aarch64-linux-gnu") {

            steps {
                echo 'building aarch64 ...'
                sh '''#!/bin/bash
                    make clean
                    ./autogen.sh
                    ./configure --prefix=$(pwd)/depends/aarch64-linux-gnu --disable-debug --disable-tests --disable-bench --enable-upnp-default CFLAGS="-O3" CXXFLAGS="-O3"
	                make -j $(nproc) HOST=aarch64-linux-gnu
                '''
            }
        }

        stage("deploy_aarch64-linux-gnu") {

            steps {
                echo 'deploy aarch64 ...'
                sh """#!/bin/bash
                    mkdir -p deploy/aarch64
                    strip -s src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt
                    cp src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt deploy/aarch64/
                    cd deploy/aarch64
                    zip ${ZIP_NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-aarch64.zip ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt
                    rm -f ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt
                """
            }
        }

        stage("build_arm-linux-gnueabihf") {

            steps {
                echo 'building aarch32 ...'
                sh '''#!/bin/bash
                    make clean
                    ./autogen.sh
                    ./configure --prefix=$(pwd)/depends/arm-linux-gnueabihf --disable-debug --disable-tests --disable-bench --enable-upnp-default CFLAGS="-O3" CXXFLAGS="-O3"
	                make -j $(nproc) HOST=arm-linux-gnueabihf
                '''
            }
        }

        stage("deploy_arm-linux-gnueabihf") {

            steps {
                echo 'deploy aarch32 ...'
                sh """#!/bin/bash
                    mkdir -p deploy/aarch32
                    strip -s src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt
                    cp src/${BASE_NAME}d src/${BASE_NAME}-cli src/${BASE_NAME}-tx src/qt/${BASE_NAME}-qt deploy/aarch32/
                    cd deploy/aarch32
                    zip ${ZIP_NAME}-\$(../${BASE_NAME}-cli --version | head -n1 | awk -F'[ -]' '{ print \$5 }' | sed s/v//)-aarch32.zip ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt
                    rm -f ${BASE_NAME}d ${BASE_NAME}-cli ${BASE_NAME}-tx ${BASE_NAME}-qt
                """
            }
        }
    }
}