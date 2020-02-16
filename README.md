# DHT
DHT11 or DHT22 Temperature Sensor monitor and recorder

Connects to MySql database called 'tempsdb' with a table called 'sensor'

Database commands as mysql root:
CREATE USER 'username'@'localhost' IDENTIFIED BY 'password';
CREATE DATABASE tempsdb;
GRANT ALL PRIVILEGES ON tempsdb.* TO 'username'@'localhost';
exit;
mysql -D tempsdb -u username -p
exit;


CREATE TABLE `tempsdb`.`sensor` ( `TIME` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP , `TYPE` VARCHAR(16) NOT NULL , `ID_NUM` INT NOT NULL , `VALUE_1` FLOAT NOT NULL , `VALUE_2` FLOAT NULL DEFAULT NULL , `VALUE_3` FLOAT NULL DEFAULT NULL , PRIMARY KEY (`TIME`)) ENGINE = InnoDB;

you can run the following command install the packages needed to compile:

sudo apt-get install libmysqlcppconn7v5 libmysqlcppconn-dev mariadb-server g++ gcc cmake wiringpi


Make a build directory and cd into it
cd DHT
mkdir build
cd build

edit the DHT/src/dht.cpp file for your database and user
nano ../src/dht.cpp

compile with:
cmake ..
make

this should give you af file called 'dht'
