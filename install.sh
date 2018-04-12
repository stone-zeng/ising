WORKING_DIR=$PWD
INCLUDE_DIR=$WORKING_DIR/ising/include

mkdir -p $INCLUDE_DIR/argagg
cd $INCLUDE_DIR/argagg
wget https://raw.githubusercontent.com/vietjtnguyen/argagg/master/include/argagg/argagg.hpp

mkdir -p $WORKING_DIR/tmp
cd $WORKING_DIR/tmp
git clone --depth=1 https://github.com/Tencent/rapidjson.git
cp -r ./rapidjson/include $INCLUDE_DIR

cd $WORKING_DIR
