#include <stdio.h>
#include <time.h>

#include "gmtime.h"

#define CONST_DATA

#ifdef CONST_DATA
/*Jesse
extern const char far* far Weeks[];
extern const char far* far Months[];
extern const char far Days[];
*/
extern const char *Weeks[];
extern const char *Months[];
extern const char Days[];
#else
const char *Weeks[7]   = {  "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
const char *Months[12] = { "Jan","Feb","Mar","Apr","May","Jun",
				           "Jul","Aug","Sep","Oct","Nov","Dec" };
const char Days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#endif

static struct tm *gmtimeX(time_t time);

// Return Date/Time in Arpanet format in passed string
char *gmt_ptime(long t)
{
	// Print out the time and date field as
	//      "DAY day MONTH year hh:mm:ss ZONE"

	register struct tm *ltm;
	static char str[40];

	// Read GMT time
	ltm = gmtimeX(t);

	// rfc 822 format
	sprintf(str,"%s, %.2d %s %04d %02d:%02d:%02d GMT",
	        Weeks[ltm->tm_wday],
	        ltm->tm_mday,
	        Months[ltm->tm_mon],
	        ltm->tm_year+1900,
	        ltm->tm_hour,
	        ltm->tm_min,
	        ltm->tm_sec);

	return(str);
}

//this function modified from BC3.1 CTIME.C
static struct tm *gmtimeX(time_t time)
{
	int      hpery;
	unsigned i;
	unsigned cumdays;
	static struct tm tmX;

	if (time < 0) time = 0;

	tmX.tm_sec = (int)(time % 60);
	time /= 60;                             //Time in minutes
	tmX.tm_min = (int)(time % 60);
	time /= 60;                             //Time in hours
	i = (unsigned)(time / (1461L * 24L));   // Number of 4 year blocks
	tmX.tm_year = (i << 2);
	tmX.tm_year+= 70;
	cumdays = 1461 * i;
	time %= 1461L * 24L;        // Hours since end of last 4 year block

	for (;;) {
		hpery = 365 * 24;
		if((tmX.tm_year & 3) == 0) hpery += 24;
		if(time < hpery) break;
		cumdays += hpery / 24;
		tmX.tm_year++;
		time -= hpery;
	}   // at end, time is number of hours into current year

	tmX.tm_isdst = 0;

	tmX.tm_hour = (int)(time % 24);
	time /= 24;             // Time in days
	tmX.tm_yday = (int)time;
	cumdays += (int)time + 4;
	tmX.tm_wday = cumdays % 7;
	time++;

	if((tmX.tm_year & 3) == 0) {
		if(time > 60) time--;
		else
			if(time == 60) {
				tmX.tm_mon = 1;
				tmX.tm_mday = 29;
				return(&tmX);
			}
	}

	for(tmX.tm_mon = 0; Days[tmX.tm_mon] < time; tmX.tm_mon++)
		time -= Days[tmX.tm_mon];
	tmX.tm_mday = (int)(time);
	return(&tmX);
}
