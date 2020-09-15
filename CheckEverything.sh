#!/bin/bash

cd Code
python3 CheckForAnyErrorsInLogFiles.py logs/SingleDO/errorlog
python3 CheckForAnyErrorsInLogFiles.py logs/MultiDO/OPRF/errorlog
python3 CheckForAnyErrorsInLogFiles.py logs/MultiDO/EqCheck/errorlog



