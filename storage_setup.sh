mkfs.ext3 /dev/sdc
cd /users/LockeZ
mkdir ssd
mount /dev/sdc /users/LockeZ/ssd
cd ssd
git clone https://github.com/ScarletGuo/Sundial.git
cd Sundial
git checkout 1pc-log
./conf
