#!/bin/bash
set -x
set -v
set -e

# Install with extra pythons
if [[ -d /opt/python/ ]]; then
    # Set LD PATH
    
    # Go through directories in order of most recetly touched;
    for PYROOT in $(ls -t /opt/python) ; do
        sudo touch "/opt/python/${PYROOT}" # Touch the directory, this means if it fails it will be built first next time
        PYBIN="/opt/python/${PYROOT}/bin"
        set -e
        if ! [[ -f "${PYBIN}/nosetests" ]]; then set -e; sudo "${PYBIN}/pip" install nose; fi
    
        rm -rf build
        "${PYBIN}/pip" wheel . --wheel-dir prefixed
        sudo "${PYBIN}/pip" uninstall -y uWebsockets || true
        sudo "${PYBIN}/pip" install --no-index -f prefixed/ uWebsockets
        # Test import
        "${PYBIN}/python" -c "import uWebSockets"
        # Run nosetests
        "${PYBIN}/python" tests/test_client.py
        mkdir -p dist/
        mv prefixed/* dist/
        set +e
        
        whl=$(ls dist -htal | grep ".whl" | awk '{print $NF}' | head -n 1)
        old=$LD_LIBRARY_PATH
        export LD_LIBRARY_PATH=/opt/openssl/lib:/usr/local/lib:$LD_LIBARY_PATH    
        (cd dist && auditwheel -vv show "$whl")
        if ! (cd dist && auditwheel repair "$whl" --wheel-dir .); then
            exit 1
        fi
        export LD_LIBRARY_PATH=$old
        unset old
        
        set -e
        sudo "${PYBIN}/pip" uninstall -y uWebsockets || true
        sudo "${PYBIN}/pip" install --no-index -f dist/ uWebsockets
        # Test import
        "${PYBIN}/python" -c "import uWebSockets"
        # Run nosetests
        "${PYBIN}/python" tests/test_client.py
        
        set +e
    done
fi

# Install with default python if it exists
if (which pip2); then rm -rf build; pip2 wheel . --wheel-dir dist; fi
if (which pip3); then rm -rf build; pip3 wheel . --wheel-dir dist; fi

