/*
 *	Code to read Data from DHT11 sensor connected to GPIO pin (use gpio readall)
 *	DHT Code from: http://www.circuitbasics.com/how-to-set-up-the-dht11-humidity-sensor-on-the-raspberry-pi/
 *	timing issue at line 45 in original code.
 *	Better one is here: http://www.uugear.com/portfolio/read-dht1122-temperature-humidity-sensor-from-raspberry-pi/
 *	timing issue at line 55 in original code
 *	timing issue: change 16 to 50
 *
 *	Added code to interface with local MySql database
 *	Rudimentary code for a Signal Handler - NOT implemented
 *  to find PID of running program
 *	use: ps aux | grep dht 
 *	use: kill 'the ID you found'
 *
*/


#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <string>
#include <iomanip>
#include <chrono>
#include <thread>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

#define MAXTIMINGS      85
#define DHTPIN          14

int dht11_dat[5] = { 0, 0, 0, 0, 0 };
sql::Driver *driver;
sql::Connection *con;
sql::Statement *stmt;
sql::ResultSet *res;

string my_schema = "tempsdb";
string my_table = "sensor";

bool valid_data = FALSE;

// Signal handler TODO make this into a daemon or service like program  UNUSED
void signalHandler(int signum) {
  cout << "Interrupt signal (" << signum <<") received.\n";
  // place cleanup and close stuff here
  if (signum != 2){
    cout << "Uh Uh Uh.  You didn't say the magic word" << endl;
  }
  //terminate the program
  else {
    cout << "Signal received";
    exit(signum);
  }
}
// Verify DB access works
int dbOpen(){
  try {
    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "username", "password");
    /* Connect to the MySQL test database */
    con->setSchema(my_schema);
  }
// Catch exceptions and print data
  catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
return EXIT_SUCCESS;
}

int dbWrite(string temperature,string humidity){
  sql::PreparedStatement *prep_stmt;
  try {
    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "aces", "abacus");
    /* Connect to the MySQL correct database */
    con->setSchema(my_schema);
//	Prepare INSERT statement
		prep_stmt = con->prepareStatement("INSERT into " + my_table + "(TYPE,ID_NUM,VALUE_1,VALUE_2) VALUES (?, ?, ?, ?)");
    prep_stmt->setString(1,"DHT11");			// Sensor TYPE label - Your choice
    prep_stmt->setInt(2,1);								// Sensor ID = 1 - Your Choice
    prep_stmt->setString(3,humidity);			// VALUE_1 = humidity value
    prep_stmt->setString(4,temperature);	// VALUE_2 = temperature value in Celsius
    prep_stmt->execute();									// Execute the SQL statement
    delete prep_stmt;											// delete the statement to clean it
    delete con;														// delete connection so it's not left open in case there is a problem
  }
// Catch exceptions and print data
  catch (sql::SQLException &e) {
    cout << "# ERR: SQLException in " << __FILE__;
    cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << " )" << endl;
  }
  return EXIT_SUCCESS;
}
// C++ program to find Current Day, Date
// and Local Time
// Used to  debug
int printTime()
{
	std::time_t t = std::time(nullptr);
	char mbstr[100];
	if (std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&t))) {
			std::cout << mbstr;
	}
	// Declaring argument for time()
	time_t tt;

	// Declaring variable to store return value of
	// localtime()
	struct tm * ti;

	// Applying time()
	time (&tt);

	// Using localtime()
	ti = localtime(&tt);
  return 0;
}
void read_dht11_dat()
{
	uint8_t laststate       = HIGH;
	uint8_t counter         = 0;
	uint8_t j               = 0, i;
	float   f , c , h;
	string f_temp, c_temp, h_temp;
	valid_data = FALSE;

	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 19 ); 										// Adding additional 1ms to MIN of 18
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	pinMode( DHTPIN, INPUT );

	for ( i = 0; i < MAXTIMINGS; i++ ){
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate ) {
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 )	{
				break;
			}
		}
		laststate = digitalRead( DHTPIN );
		if ( counter == 255 )
			break;
		if ( (i >= 4) && (i % 2 == 0) ) {
			dht11_dat[j / 8] <<= 1;
			if ( counter > 50 )										// Timing issue is here. changed 16 to 50
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}

	if ( (j >= 40) &&
			 (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) ) {
		c = stof(to_string(dht11_dat[2]) + "." + to_string(dht11_dat[3]));
		h = stof(to_string(dht11_dat[0]) + "." + to_string(dht11_dat[1]));
		f = c * 9. / 5. + 32;
		c_temp = to_string(c);
		f_temp = to_string(f);
		h_temp = to_string(dht11_dat[0]) + "." + to_string(dht11_dat[1]);
//	Various print routines for debugging
//		printTime();
//		printf( " %d.%d %d.%d %.1f\n", dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3], f );
//		cout << endl << "h: " << h_temp << " c: " << c_temp << " f: " << f_temp << endl;	
		dbWrite(c_temp, h_temp);
		valid_data = TRUE;
	}else  {
//			Various print routines for debugging
//			printTime();
//			printf( " %d.%d %d.%d %.1f\n", 0, 0, 0, 0, 0 );
//			printf( "Data not good, skip\n" );
		return;
    }
}


int main( void )
{
  using namespace std::this_thread;     // sleep_for, sleep_until
  using namespace std::chrono_literals; // ns, us, ms, s, min, h, etc.
  using std::chrono::system_clock;

// Get Process ID
  cout << "\nCurrent process id of Process : " << getpid() << endl;
//  int pid = fork();
//  if (pid == 0){
//    cout << "\nCurrent process id of Process : " << getpid() << endl;
//  }

// register signal SIGINT and signal handler
  signal(SIGINT, signalHandler);

//	printf( "Raspberry Pi wiringPi DHT11 Temperature test program\n" );

	if ( wiringPiSetup() == -1 )
                exit( 1 );
  if (dbOpen() != EXIT_SUCCESS){
    cout << "Error opening DataBase" << endl;
  }
	while ( TRUE )
	{
//        printTime();
		while (!valid_data) {
			read_dht11_dat();
		}
		valid_data = FALSE;
		// Different delay possibilities
//		delay( 2000 );
//		sleep(60);
		sleep_until(system_clock::now() + 5min);
	}
	return(0);
}
