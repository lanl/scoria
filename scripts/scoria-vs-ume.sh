#!/bin/bash
usage() {
	echo "Usage ./scoria-vs-ume.sh -f <absolute-path-to-input-file> [-c 'caliconfig'] [-n 'ranks'] [-s 'scoria-root']
		-f : Specify the absolute path to an Ume input file
		-c : Optional, CALI_CONFIG environment variable (Default: 'runtime-report(output=report.log)')
		-n : Optional, Specify the number of MPI ranks to run Ume with (Default: 1)
                -s : Optional, Root directory of the Scoria project (Default: pwd)
		-p : Optional, List of PAPI hardware counters to collect"
}

CALI_CONFIG="runtime-report(output=report.log)"
RANKS=1
SCORIA_OPUS=`pwd`

while getopts "f:c:n:s:p:" opt; do
	case $opt in
		c) CALI_CONFIG=$OPTARG
		;;
		f) INPUT=$OPTARG
		;;
		n) RANKS=$OPTARG
		;;
		s) SCORIA_OPUS=$OPTARG
		;;
		p)
				export CALI_SERVICES_ENABLE=event:papi:trace:recorder
				export CALI_PAPI_COUNTERS="${OPTARG}"
				export CALI_RECORDER_FILENAME=papi-counters-${CALI_PAPI_COUNTERS}.cali
				export CALI_PAPI_ENABLE_MULTIPLEXING="true"
		;;
		h) usage; exit 1
		;;
	esac
done

if [ -z "${INPUT}" ]; then
	usage
	exit 1
fi

echo "Running scoria-vs-ume.sh"
echo "CALI_CONFIG: ${CALI_CONFIG}"
echo "INPUT: ${INPUT}"
echo "RANKS: ${RANKS}"
echo "SCORIA_ROOT: ${SCORIA_OPUS}"

cd $SCORIA_OPUS

SCORIA_BUILD=$SCORIA_OPUS/caliper_build
mkdir -p $SCORIA_BUILD

UME_OPUS=$SCORIA_OPUS/clients/UME
UME_BUILD=$UME_OPUS/caliper_build
mkdir -p $UME_BUILD

export CALI_CONFIG=${CALI_CONFIG}

scoria_ume_mpi=$SCORIA_BUILD/clients/UME/src/ume_mpi
ume_mpi=$UME_BUILD/src/ume_mpi

# Ume + Scoria
cd $SCORIA_BUILD
cmake -DUSE_CALIPER=ON -DScoria_REQUIRE_AVX=ON -DUSE_MPI=ON -DCMAKE_BUILD_TYPE=Release ../
make -j

RELINPUT=`realpath --relative-to=${PWD} $INPUT`
mpirun -n $RANKS $scoria_ume_mpi $RELINPUT

# Ume
cd $UME_BUILD
cmake -DUSE_CALIPER=ON -DUSE_MPI=ON -DCMAKE_BUILD_TYPE=Release ../
make -j

RELINPUT=`realpath --relative-to=${PWD} $INPUT`
mpirun --bind-to core -n $RANKS $ume_mpi $RELINPUT
