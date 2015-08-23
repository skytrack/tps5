
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    unsigned int ilog_10( unsigned int aI );

    #define LONG_DIGITS 10

    unsigned int lllog_10( unsigned long long aLL );

    #define LLONG_DIGITS 20

    char * ustoa( char *aBuf, unsigned short aShort );

    char * stoa( char *aBuf, short aShort );

    char * uitoa( char *aBuf, unsigned int anInt );

    char * uitoa2( char *aBuf, unsigned int anInt );

    //char * itoa( char *aBuf, int anInt );

    char * ulltoa( char *aBuf, unsigned long long aLL );

    char * ulltoa2( char *aBuf, unsigned long long aLL );

    char * ulltoa3( char *aBuf, unsigned long long aLL );

    char * ulltoa4( char *aBuf, unsigned long long aLL );

    char * lltoa( char *aBuf, long long aLL );

    char * dtoa( char *aBuf, double aD );
#ifdef __cplusplus
};
#endif /* __cplusplus */
