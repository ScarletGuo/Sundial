mkfs.ext3 /dev/sdc
cd /users/LockeZ
mkdir ssd
mount /dev/sdc /users/LockeZ/ssd
cd ssd
git clone https://github.com/ScarletGuo/Sundial.git
cd Sundial
git checkout 1pc-log
cd /etc/ld.so.conf.d
echo "/users/LockeZ/ssd/Sundial/libs/" | sudo tee other.conf
sudo /sbin/ldconfig
cd /users/LockeZ/ssd/Sundial/
git config --global credential.helper store
git config --global user.name "Xinyu Zeng"
git config --global user.email "xzeng@cs.wisc.edu"
cp vimsetting ~/.vimrc
sudo apt update
sudo apt -y install python3-pip
pip3 install pandas

