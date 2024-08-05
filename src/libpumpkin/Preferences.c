#include <PalmOS.h>

#include "thread.h"
#include "mutex.h"
#include "sys.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define PALMOS_MODULE "Prefs"

#define UNSAVED_PREFS "Unsaved Preferences"
#define SAVED_PREFS   "Saved Preferences"

static mutex_t *mutex;
static SystemPreferencesType prefs;

typedef struct {
  char *descr;
  UInt16 code;
  char *name;
} locale_map_t;

#define NUM_COUNTRIES 239

static locale_map_t countries[NUM_COUNTRIES] = {
  { "AU", 0, "Australia" },
  { "AT", 1, "Austria" },
  { "BE", 2, "Belgium" },
  { "BR", 3, "Brazil" },
  { "CA", 4, "Canada" },
  { "DK", 5, "Denmark" },
  { "FI", 6, "Finland" },
  { "FR", 7, "France" },
  { "DE", 8, "Germany" },
  { "HK", 9, "Hong Kong" },
  { "IS", 10, "Iceland" },
  { "IE", 11, "Ireland" },
  { "IT", 12, "Italy" },
  { "JP", 13, "Japan" },
  { "LU", 14, "Luxembourg" },
  { "MX", 15, "Mexico" },
  { "NL", 16, "Netherlands" },
  { "NZ", 17, "New Zealand" },
  { "NO", 18, "Norway" },
  { "ES", 19, "Spain" },
  { "SE", 20, "Sweden" },
  { "CH", 21, "Switzerland" },
  { "GB", 22, "United Kingdom" },
  { "US", 23, "United States" },
  { "IN", 24, "India" },
  { "ID", 25, "Indonesia" },
  { "KR", 26, "Republic of Korea" },
  { "MY", 27, "Malaysia" },
  { "CN", 28, "China" },
  { "PH", 29, "Philippines" },
  { "SG", 30, "Singapore" },
  { "TH", 31, "Thailand" },
  { "TW", 32, "Taiwan" },
  { "AD", 33, "Andorra" },
  { "AE", 34, "United Arab Emirates" },
  { "AF", 35, "Afghanistan" },
  { "AG", 36, "Antigua and Barbuda" },
  { "AI", 37, "Anguilla" },
  { "AL", 38, "Albania" },
  { "AM", 39, "Armenia" },
  { "AN", 40, "Netherlands Antilles" },
  { "AO", 41, "Angola" },
  { "AQ", 42, "Antarctica" },
  { "AR", 43, "Argentina" },
  { "AS", 44, "American Samoa" },
  { "AW", 45, "Aruba" },
  { "AZ", 46, "Azerbaijan" },
  { "BA", 47, "Bosnia and Herzegovina" },
  { "BB", 48, "Barbados" },
  { "BD", 49, "Bangladesh" },
  { "BF", 50, "Burkina Faso" },
  { "BG", 51, "Bulgaria" },
  { "BH", 52, "Bahrain" },
  { "BI", 53, "Burundi" },
  { "BJ", 54, "Benin" },
  { "BM", 55, "Bermuda" },
  { "BN", 56, "Brunei Darussalam" },
  { "BO", 57, "Bolivia" },
  { "BS", 58, "Bahamas" },
  { "BT", 59, "Bhutan" },
  { "BV", 60, "Bouvet Island" },
  { "BW", 61, "Botswana" },
  { "BY", 62, "Belarus" },
  { "BZ", 63, "Belize" },
  { "CC", 64, "Cocos Islands" },
  { "CD", 65, "Democratic Republic of the Congo" },
  { "CF", 66, "Central African Republic" },
  { "CG", 67, "Congo" },
  { "CI", 68, "Ivory Coast" },
  { "CK", 69, "Cook Islands" },
  { "CL", 70, "Chile" },
  { "CM", 71, "Cameroon" },
  { "CO", 72, "Columbia" },
  { "CR", 73, "Costa Rica" },
  { "CU", 74, "Cuba" },
  { "CV", 75, "Cape Verde" },
  { "CX", 76, "Christmas Island" },
  { "CY", 77, "Cyprus" },
  { "CZ", 78, "Czech Republic" },
  { "DJ", 79, "Djibouti" },
  { "DM", 80, "Dominica" },
  { "DO", 81, "Dominican Republic" },
  { "DZ", 82, "Algeria" },
  { "EC", 83, "Ecuador" },
  { "EE", 84, "Estonia" },
  { "EG", 85, "Egypt" },
  { "EH", 86, "Western Sahara" },
  { "ER", 87, "Eritrea" },
  { "ET", 88, "Ethiopia" },
  { "FJ", 89, "Fiji" },
  { "FK", 90, "Falkland Islands" },
  { "FM", 91, "Micronesia" },
  { "FO", 92, "Faeroe Islands" },
  { "FX", 93, "Metropolitan France" },
  { "GA", 94, "Gabon" },
  { "GD", 95, "Grenada" },
  { "GE", 96, "Georgia" },
  { "GF", 97, "French Guiana" },
  { "GH", 98, "Ghana" },
  { "GI", 99, "Gibraltar" },
  { "GL", 100, "Greenland" },
  { "GM", 101, "Gambia" },
  { "GN", 102, "Guinea" },
  { "GP", 103, "Guadeloupe" },
  { "GQ", 104, "Equatorial Guinea" },
  { "GR", 105, "Greece" },
  { "GS", 106, "South Georgia and the South Sandwich Islands" },
  { "GT", 107, "Guatemala" },
  { "GU", 108, "Guam" },
  { "GW", 109, "GuineaBisseu" },
  { "GY", 110, "Guyana" },
  { "HM", 111, "Heard and McDonald Islands" },
  { "HN", 112, "Honduras" },
  { "HR", 113, "Croatia" },
  { "HT", 114, "Haiti" },
  { "HU", 115, "Hungary" },
  { "IL", 116, "Israel" },
  { "IO", 117, "British Indian Ocean Territory" },
  { "IQ", 118, "Iraq" },
  { "IR", 119, "Iran" },
  { "JM", 120, "Jamaica" },
  { "JO", 121, "Jordan" },
  { "KE", 122, "Kenya" },
  { "KG", 123, "Kyrgyzstan" },
  { "KH", 124, "Cambodia" },
  { "KI", 125, "Kiribati" },
  { "KM", 126, "Comoros" },
  { "KN", 127, "StKitts and Nevis" },
  { "KP", 128, "Democratic Peoples Republic of Korea" },
  { "KW", 129, "Kuwait" },
  { "KY", 130, "Cayman Islands" },
  { "KK", 131, "Kazakhstan" },
  { "LA", 132, "Laos" },
  { "LB", 133, "Lebanon" },
  { "LC", 134, "StLucia" },
  { "LI", 135, "Liechtenstein" },
  { "LK", 136, "SriLanka" },
  { "LR", 137, "Liberia" },
  { "LS", 138, "Lesotho" },
  { "LT", 139, "Lithuania" },
  { "LV", 140, "Latvia" },
  { "LY", 141, "Libya" },
  { "MA", 142, "Morrocco" },
  { "MC", 143, "Monaco" },
  { "MD", 144, "Moldova" },
  { "MG", 145, "Madagascar" },
  { "MH", 146, "Marshall Islands" },
  { "MK", 147, "Macedonia" },
  { "ML", 148, "Mali" },
  { "MM", 149, "Myanmar" },
  { "MN", 150, "Mongolia" },
  { "MO", 151, "Macau" },
  { "MP", 152, "Northern Mariana Islands" },
  { "MQ", 153, "Martinique" },
  { "MR", 154, "Mauritania" },
  { "MS", 155, "Montserrat" },
  { "MT", 156, "Malta" },
  { "MU", 157, "Mauritius" },
  { "MV", 158, "Maldives" },
  { "MW", 159, "Malawi" },
  { "MZ", 160, "Mozambique" },
  { "NA", 161, "Namibia" },
  { "NC", 162, "NewCaledonia" },
  { "NE", 163, "Niger" },
  { "NF", 164, "Norfolk Island" },
  { "NG", 165, "Nigeria" },
  { "NI", 166, "Nicaragua" },
  { "NP", 167, "Nepal" },
  { "NR", 168, "Nauru" },
  { "NU", 169, "Niue" },
  { "OM", 170, "Oman" },
  { "PA", 171, "Panama" },
  { "PE", 172, "Peru" },
  { "PF", 173, "French Polynesia" },
  { "PG", 174, "Papua New Guinea" },
  { "PK", 175, "Pakistan" },
  { "PL", 176, "Poland" },
  { "PM", 177, "St Pierre and Miquelon" },
  { "PN", 178, "Pitcairn" },
  { "PR", 179, "Puerto Rico" },
  { "PT", 180, "Portugal" },
  { "PW", 181, "Palau" },
  { "PY", 182, "Paraguay" },
  { "QA", 183, "Qatar" },
  { "RE", 184, "Reunion" },
  { "RO", 185, "Romania" },
  { "RU", 186, "Russian Federation" },
  { "RW", 187, "Rwanda" },
  { "SA", 188, "Saudi Arabia" },
  { "SB", 189, "Solomon Islands" },
  { "SC", 190, "Seychelles" },
  { "SD", 191, "Sudan" },
  { "SH", 192, "StHelena" },
  { "SI", 193, "Slovenia" },
  { "SJ", 194, "Svalbard and JanMayen Islands" },
  { "SK", 195, "Slovakia" },
  { "SL", 196, "Sierra Leone" },
  { "SM", 197, "SanMarino" },
  { "SN", 198, "Senegal" },
  { "SO", 199, "Somalia" },
  { "SR", 200, "Suriname" },
  { "ST", 201, "Sao Tome and Principe" },
  { "SV", 202, "El Salvador" },
  { "SY", 203, "Syran Arab Republic" },
  { "SZ", 204, "Swaziland" },
  { "TC", 205, "Turks and Caicos Islands" },
  { "TD", 206, "Chad" },
  { "TF", 207, "French Southern Territories" },
  { "TG", 208, "Togo" },
  { "TJ", 209, "Tajikistan" },
  { "TK", 210, "Tokelau" },
  { "TM", 211, "Turkmenistan" },
  { "TN", 212, "Tunisia" },
  { "TO", 213, "Tonga" },
  { "TP", 214, "EastTimor" },
  { "TR", 215, "Turkey" },
  { "TT", 216, "Trinidad and Tobago" },
  { "TV", 217, "Tuvalu" },
  { "TZ", 218, "Tanzania" },
  { "UA", 219, "Ukraine" },
  { "UG", 220, "Uganda" },
  { "UM", 221, "United States Minor Outlying Islands" },
  { "UY", 222, "Uruguay" },
  { "UZ", 223, "Uzbekistan" },
  { "VA", 224, "HolySee" },
  { "VC", 225, "StVincent and the Grenadines" },
  { "VE", 226, "Venezuela" },
  { "VG", 227, "British Virgin Islands" },
  { "VI", 228, "US Virgin Islands" },
  { "VN", 229, "VietNam" },
  { "VU", 230, "Vanuatu" },
  { "WF", 231, "Wallis and Futuna Islands" },
  { "WS", 232, "Samoa" },
  { "YE", 233, "Yemen" },
  { "YT", 234, "Mayotte" },
  { "YU", 235, "Yugoslavia" },
  { "ZA", 236, "South Africa" },
  { "ZM", 237, "Zambia" },
  { "ZW", 238, "Zimbabwe" }
};

#define NUM_LANGUAGES 137

static locale_map_t languages[NUM_LANGUAGES] = {
  { "EN", 0, "English" },
  { "FR", 1, "French" },
  { "DE", 2, "German" },
  { "IT", 3, "Italian" },
  { "ES", 4, "Spanish" },
  { "UTF8", 5, "UTF8" }, // was "Unused"
  { "JA", 6, "Japanese" },
  { "NL", 7, "Dutch" },
  { "AA", 8, "Afar" },
  { "AB", 9, "Abkhazian" },
  { "AF", 10, "Afrikaans" },
  { "AM", 11, "Amharic" },
  { "AR", 12, "Arabic" },
  { "AS", 13, "Assamese" },
  { "AY", 14, "Aymara" },
  { "AZ", 15, "Azerbaijani" },
  { "BA", 16, "Bashkir" },
  { "BE", 17, "Byelorussian" },
  { "BG", 18, "Bulgarian" },
  { "BH", 19, "Bihari" },
  { "BI", 20, "Bislama" },
  { "BN", 21, "Bengali" },
  { "BO", 22, "Tibetan" },
  { "BR", 23, "Breton" },
  { "CA", 24, "Catalan" },
  { "CO", 25, "Corsican" },
  { "CS", 26, "Czech" },
  { "CY", 27, "Welsh" },
  { "DA", 28, "Danish" },
  { "DZ", 29, "Bhutani" },
  { "EL", 30, "Greek" },
  { "EO", 31, "Esperanto" },
  { "ET", 32, "Estonian" },
  { "EU", 33, "Basque" },
  { "FA", 34, "Persian" },
  { "FI", 35, "Finnish" },
  { "FJ", 36, "Fiji" },
  { "FO", 37, "Faroese" },
  { "FY", 38, "Frisian" },
  { "GA", 39, "Irish" },
  { "GD", 40, "Scots Gaelic" },
  { "GL", 41, "Galician" },
  { "GN", 42, "Guarani" },
  { "GU", 43, "Gujarati" },
  { "HA", 44, "Hausa" },
  { "HI", 45, "Hindi" },
  { "HR", 46, "Croatian" },
  { "HU", 47, "Hungarian" },
  { "HY", 48, "Armenian" },
  { "IA", 49, "Interlingua" },
  { "IE", 50, "Interlingue" },
  { "IK", 51, "Inupiak" },
  { "IN", 52, "Indonesian" },
  { "IS", 53, "Icelandic" },
  { "IW", 54, "Hebrew" },
  { "JI", 55, "Yiddish" },
  { "JW", 56, "Javanese" },
  { "KA", 57, "Georgian" },
  { "KK", 58, "Kazakh" },
  { "KL", 59, "Greenlandic" },
  { "KM", 60, "Cambodian" },
  { "KN", 61, "Kannada" },
  { "KO", 62, "Korean" },
  { "KS", 63, "Kashmiri" },
  { "KU", 64, "Kurdish" },
  { "KY", 65, "Kirghiz" },
  { "LA", 66, "Latin" },
  { "LN", 67, "Lingala" },
  { "LO", 68, "Laothian" },
  { "LT", 69, "Lithuanian" },
  { "LV", 70, "Latvian" },
  { "MG", 71, "Malagasy" },
  { "MI", 72, "Maori" },
  { "MK", 73, "Macedonian" },
  { "ML", 74, "Malayalam" },
  { "MN", 75, "Mongolian" },
  { "MO", 76, "Moldavian" },
  { "MR", 77, "Marathi" },
  { "MS", 78, "Malay" },
  { "MT", 79, "Maltese" },
  { "MY", 80, "Burmese" },
  { "NA", 81, "Nauru" },
  { "NE", 82, "Nepali" },
  { "NO", 83, "Norwegian" },
  { "OC", 84, "Occitan" },
  { "OM", 85, "Afan" },
  { "OR", 86, "Oriya" },
  { "PA", 87, "Punjabi" },
  { "PL", 88, "Polish" },
  { "PS", 89, "Pashto" },
  { "PT", 90, "Portuguese" },
  { "QU", 91, "Quechua" },
  { "RM", 92, "Rhaeto Romance" },
  { "RN", 93, "Kurundi" },
  { "RO", 94, "Romanian" },
  { "RU", 95, "Russian" },
  { "RW", 96, "Kinyarwanda" },
  { "SA", 97, "Sanskrit" },
  { "SD", 98, "Sindhi" },
  { "SG", 99, "Sangho" },
  { "SH", 100, "Serbo Croatian" },
  { "SI", 101, "Singhalese" },
  { "SK", 102, "Slovak" },
  { "SL", 103, "Slovenian" },
  { "SM", 104, "Samoan" },
  { "SN", 105, "Shona" },
  { "SO", 106, "Somali" },
  { "SQ", 107, "Albanian" },
  { "SR", 108, "Serbian" },
  { "SS", 109, "Siswati" },
  { "ST", 110, "Sesotho" },
  { "SU", 111, "Sudanese" },
  { "SV", 112, "Swedish" },
  { "SW", 113, "Swahili" },
  { "TA", 114, "Tamil" },
  { "TE", 115, "Telugu" },
  { "TG", 116, "Tajik" },
  { "TH", 117, "Thai" },
  { "TI", 118, "Tigrinya" },
  { "TK", 119, "Turkmen" },
  { "TL", 120, "Tagalog" },
  { "TN", 121, "Setswana" },
  { "TO", 122, "Tonga" },
  { "TR", 123, "Turkish" },
  { "TS", 124, "Tsonga" },
  { "TT", 125, "Tatar" },
  { "TW", 126, "Twi" },
  { "UK", 127, "Ukrainian" },
  { "UR", 128, "Urdu" },
  { "UZ", 129, "Uzbek" },
  { "VI", 130, "Vietnamese" },
  { "VO", 131, "Volapuk" },
  { "WO", 132, "Wolof" },
  { "XH", 133, "Xhosa" },
  { "YO", 134, "Yoruba" },
  { "ZH", 135, "Chinese" },
  { "ZU", 136, "Zulu" }
};

static Int16 mapCountry(char *country) {
  UInt16 i;

  for (i = 0; i < NUM_COUNTRIES; i++) {
    if (StrNCaselessCompare(country, countries[i].descr, 2) == 0) {
      return countries[i].code;
    }
  }

  return -1;
}

static Int16 mapLanguage(char *language) {
  UInt16 i;

  for (i = 0; i < NUM_LANGUAGES; i++) {
    if (StrNCaselessCompare(language, languages[i].descr, 2) == 0) {
      return languages[i].code;
    }
  }

  return -1;
}

static int16_t initTimePrefs(SystemPreferencesType *prefs, int *_dst) {
  sys_tm_t tm;
  uint64_t dt, tl;
  int64_t sdt;
  int dst;

  dt = sys_time();
  sys_gmtime(&dt, &tm);
  sdt = (int64_t)dt;
  tl = (int64_t)sys_timelocal(&tm);
  sdt -= tl;
  sdt /= 60;
  dst = sys_isdst() ? 60 : 0;
  sdt -= dst;

  debug(DEBUG_INFO, PALMOS_MODULE, "setting prefs timeZone %d (dst %d)", sdt, dst);
  prefs->timeZone = sdt;
  prefs->daylightSavingAdjustment = dst;
  prefs->minutesWestOfGMT = prefs->timeZone; // XXX minutesWestOfGMT is UInt32

  if (_dst) *_dst = dst;
  return (int16_t)sdt;
}

static void initPrefs(SystemPreferencesType *prefs) {
  LmLocaleType systemLocale, aux;
  char countryName[kMaxCountryNameLen+1];
  char country[8], language[16];
  UInt16 index;
  Int16 n;
  int16_t dt;
  int dst;
  Boolean found;

  prefs->version = 9;                             // Version of preference info

  // International preferences
  prefs->country = cUnitedStates;                 // Country the device is in (see PalmLocale.h)
  prefs->dateFormat = dfMDYWithSlashes;           // Format to display date in
  prefs->longDateFormat = dfMDYLongWithComma;     // Format to display date in
  prefs->weekStartDay = sunday;                   // Sunday or Monday
  prefs->timeFormat = tfColonAMPM;                // Format to display time in
  prefs->numberFormat = nfCommaPeriod;            // Format to display numbers in

  prefs->autoOffDuration = 2;                     // Time period in minutes before shutting off (use autoOffDurationSecs instead).
  prefs->sysSoundLevelV20 = slOff;                // slOn or slOff - error beeps and other non-alarm/game sounds
  prefs->gameSoundLevelV20 = slOff;               // slOn or slOff - game sound effects
  prefs->alarmSoundLevelV20 = slOff;              // slOn or slOff - alarm sound effects
  prefs->hideSecretRecords = false;               // True to not display records with their secret bit attribute set
  prefs->deviceLocked = false;                    // Device locked until the system password is entered
  prefs->localSyncRequiresPassword = false;       // User must enter password on Pilot
  prefs->remoteSyncRequiresPassword = false;      // User must enter password on Pilot
  prefs->sysPrefFlags = 0;                        // XXX Miscellaneous system pref flags copied into the global GSysPrefFlags at boot time.
  prefs->sysBatteryKind = sysBatteryKindAlkaline; // The type of batteries installed. This is copied into the globals GSysbatteryKind at boot time.
  prefs->reserved1 = 0;
  prefs->minutesWestOfGMT = 0;                    // XXX minutes west of Greenwich
  prefs->daylightSavings = dsNone;                // XXX Type of daylight savings correction
  prefs->reserved2 = 0;
  prefs->ronamaticChar = 0;                       // XXX character to generate from ronamatic stroke. Typically it popups the onscreen keyboard.
  prefs->hard1CharAppCreator = 0;                 // XXX creator of application to launch in response to the hard button #1. Used by SysHandleEvent.
  prefs->hard2CharAppCreator = 0;                 // XXX creator of application to launch in response to the hard button #2. Used by SysHandleEvent.
  prefs->hard3CharAppCreator = 0;                 // XXX creator of application to launch in response to the hard button #3. Used by SysHandleEvent.
  prefs->hard4CharAppCreator = 0;                 // XXX creator of application to launch in response to the hard button #4. Used by SysHandleEvent.
  prefs->calcCharAppCreator = 0;                  // XXX creator of application to launch in response to the Calculator icon. Used by SysHandleEvent.
  prefs->hardCradleCharAppCreator = 0;            // XXX creator of application to launch in response to the Cradle button. Used by SysHandleEvent.
  prefs->launcherCharAppCreator = 0;              // XXX creator of application to launch in response to the launcher button. Used by SysHandleEvent.
  prefs->hardCradle2CharAppCreator = 0;           // XXX creator of application to launch in response to the 2nd Cradle button. Used by SysHandleEvent.
  prefs->animationLevel = alOff;                  // amount of animation to display
  prefs->maskPrivateRecords = false;              // Only meaningful if hideSecretRecords is true. true to show a grey placeholder box for secret records.

  // Additions for PalmOS 3.0:
  prefs->sysSoundVolume = 0;                      // system amplitude (0 - sndMaxAmp) - taps, beeps
  prefs->gameSoundVolume = 0;                     // game amplitude (0 - sndMaxAmp) - explosions
  prefs->alarmSoundVolume = 0;                    // alarm amplitude (0 - sndMaxAmp)
  prefs->beamReceive = false;                     // False turns off IR sniffing, sends still work.
  prefs->calibrateDigitizerAtReset = false;       // True makes the user calibrate at soft reset time
  prefs->systemKeyboardID = 0;                    // XXX ID of the preferred keyboard resource
  prefs->defSerialPlugIn = 0;                     // XXX creator ID of the default serial plug-in

  // Additions for PalmOS 3.1:
  prefs->stayOnWhenPluggedIn = true;              // don't sleep after timeout when using line current
  prefs->stayLitWhenPluggedIn = true;             // keep backlight on when not sleeping on line current

  // Additions for PalmOS 3.2:
  prefs->antennaCharAppCreator = 0;               // creator of application to launch in response to the antenna key. Used by SysHandleEvent.

  // Additions for PalmOS 3.5:
  prefs->measurementSystem = unitsMetric;         // metric, english, etc.
  prefs->reserved3 = 0;
  prefs->autoOffDurationSecs = 0;                 // Time period in seconds before shutting off.

  // Additions for PalmOS 4.0:
  prefs->timeZone = 0;                            // XXX minutes east of Greenwich
  prefs->daylightSavingAdjustment = 0;            // XXX current daylight saving correction in minutes
  prefs->timeZoneCountry = cUnitedStates;         // XXX country used to specify time zone.
  prefs->autoLockType = never;                    // Never, on power off, after preset delay or at preset time
  prefs->autoLockTime = 0;                        // Auto lock preset time or delay.
  prefs->autoLockTimeFlag = false;                // XXX For Minutes or Hours.
  prefs->language = lEnglish;                     // Language spoken in country selected via Setup app/Formats panel
  prefs->attentionFlags = 0;                      // User prefs for getting user's attention
  prefs->defaultAppCreator = 0;                   // XXX Creator of the default "safe" app that is launched on a reset.

  // Additions for PalmOS 5.0:
  prefs->defFepPlugInCreator = 0;                 // XXX Creator of the default Fep plug-in on a reset.
  prefs->colorThemeID = 0;                        // XXX Resource ID of the color theme.

  // Additions for PalmOS 5.3
  prefs->handedness = 0;                          // XXX
  prefs->HWRCreator = 0;                          // creator of the HWR library

  systemLocale.country = lmAnyCountry;
  systemLocale.language = lmAnyLanguage;
  found = false;

  if (sys_country(country, sizeof(country)) == 0) {
    debug(DEBUG_INFO, PALMOS_MODULE, "host country is \"%s\"", country);
    n = mapCountry(country);
    if (n != -1) {
      debug(DEBUG_INFO, PALMOS_MODULE, "country code for \"%s\" is %d", country, n);
      systemLocale.country = n;
    } else {
      debug(DEBUG_ERROR, PALMOS_MODULE, "country code for \"%s\" not found", country);
    }
  } else {
    debug(DEBUG_ERROR, PALMOS_MODULE, "host country information not available");
  }

  if (sys_language(language, sizeof(language)) == 0) {
    debug(DEBUG_INFO, PALMOS_MODULE, "host language is \"%s\"", language);
    n = mapLanguage(language);
    if (n != -1) {
      debug(DEBUG_INFO, PALMOS_MODULE, "language code for \"%s\" is %d", language, n);
      systemLocale.language = n;
    } else {
      debug(DEBUG_ERROR, PALMOS_MODULE, "language code for \"%s\" not found", language);
    }
  } else {
    debug(DEBUG_ERROR, PALMOS_MODULE, "host language information not available");
  }

  if (systemLocale.country != lmAnyCountry && systemLocale.language != lmAnyLanguage) {
    // both country and language are available
    if (LmLocaleToIndex(&systemLocale, &index) == errNone) {
      debug(DEBUG_INFO, PALMOS_MODULE, "found locale index %d based on country %d and language %d", index, systemLocale.country, systemLocale.language);
      found = true;
    }
  } else if (systemLocale.country != lmAnyCountry) {
    // only country is available
    if (LmLocaleToIndex(&systemLocale, &index) == errNone) {
      debug(DEBUG_INFO, PALMOS_MODULE, "found locale index %d based on country %d", index, systemLocale.country);
      found = true;
    }
  }

  dt = initTimePrefs(prefs, &dst);

  if (!found) {
    if (systemLocale.language == lmAnyLanguage) {
      index = 0;
      if (LmTimeZoneToIndex(dt, &index) == errNone) {
        debug(DEBUG_INFO, PALMOS_MODULE, "inferred locale index %d based on timeZone %d (dst %d)", index, dt, dst);
        found = true;
      }
    } else {
      for (index = 0;; index++) {
        if (LmTimeZoneToIndex(dt, &index) != errNone) break;
        LmGetLocaleSetting(index, lmChoiceLocale, &aux, sizeof(LmLocaleType));
        if (aux.language == systemLocale.language) {
          debug(DEBUG_INFO, PALMOS_MODULE, "inferred locale index %d based on language %d and timeZone %d (dst %d)", index, systemLocale.language, dt, dst);
          found = true;
          break;
        }
      }
      if (!found) {
        debug(DEBUG_ERROR, PALMOS_MODULE, "could not infer locale based on language %d timeZone %d (dst %d)", systemLocale.language, dt, dst);
      }
    }

    if (!found) {
      aux.language = lmAnyLanguage;
      aux.country = cUnitedStates;
      if (LmLocaleToIndex(&aux, &index) == errNone) {
        debug(DEBUG_INFO, PALMOS_MODULE, "defaulting to US locale index %d", index);
        systemLocale.country = aux.country;
        systemLocale.language = aux.language;
        found = true;
      } else {
        debug(DEBUG_ERROR, PALMOS_MODULE, "could not find any suitable locale");
      }
    }
  }

  if (found) {
    MemSet(countryName, kMaxCountryNameLen+1, 0);
    LmGetLocaleSetting(index, lmChoiceCountryName, countryName, kMaxCountryNameLen+1);
    LmGetLocaleSetting(index, lmChoiceLocale, &systemLocale, sizeof(LmLocaleType));

    debug(DEBUG_INFO, PALMOS_MODULE, "setting locale for country %d (%s) and language %d", systemLocale.country, countryName, systemLocale.language);
    OmSetSystemLocale(&systemLocale);

    prefs->country = systemLocale.country;
    prefs->timeZoneCountry = systemLocale.country;
    prefs->language = systemLocale.language;

    LmGetLocaleSetting(index, lmChoiceDateFormat, &prefs->dateFormat, sizeof(prefs->dateFormat));
    LmGetLocaleSetting(index, lmChoiceLongDateFormat, &prefs->longDateFormat, sizeof(prefs->longDateFormat));
    LmGetLocaleSetting(index, lmChoiceTimeFormat, &prefs->timeFormat, sizeof(prefs->timeFormat));
    LmGetLocaleSetting(index, lmChoiceNumberFormat, &prefs->numberFormat, sizeof(prefs->numberFormat));
    LmGetLocaleSetting(index, lmChoiceWeekStartDay, &prefs->weekStartDay, sizeof(prefs->weekStartDay));
    LmGetLocaleSetting(index, lmChoiceMeasurementSystem, &prefs->measurementSystem, sizeof(prefs->measurementSystem));

  } else if (systemLocale.country != lmAnyCountry) {
    debug(DEBUG_INFO, PALMOS_MODULE, "setting prefs for country %d", systemLocale.country);
    prefs->country = systemLocale.country;
    prefs->timeZoneCountry = systemLocale.country;

  } else if (systemLocale.language != lmAnyLanguage) {
    debug(DEBUG_INFO, PALMOS_MODULE, "setting prefs for language %d", systemLocale.language);
    prefs->language = systemLocale.language;
  }
}

int PrefInitModule(void) {
  UInt16 size;

  mutex = mutex_create("prefs");
  size = sizeof(SystemPreferencesType);
  MemSet(&prefs, size, 0);

  if (pumpkin_get_preference(BOOT_CREATOR, 1, &prefs, size, true) == 0) {
    debug(DEBUG_INFO, PALMOS_MODULE, "initializing system preferences");
    initPrefs(&prefs);
    pumpkin_set_preference(BOOT_CREATOR, 1, &prefs, size, true);
  } else {
    debug(DEBUG_INFO, PALMOS_MODULE, "loading system preferences");
  }

  if (DmFindDatabase(0, SAVED_PREFS) == 0) {
    DmCreateDatabase(0, SAVED_PREFS, sysFileCSystem, sysFileTSavedPreferences, true);
  }

  if (DmFindDatabase(0, UNSAVED_PREFS) == 0) {
    DmCreateDatabase(0, UNSAVED_PREFS, sysFileCSystem, sysFileTPreferences, true);
  }

  return 0;
}

int PrefFinishModule(void) {
  DmOpenRef dbRef;
  UInt16 num;

  debug(DEBUG_INFO, PALMOS_MODULE, "saving system preferences");
  pumpkin_set_preference(BOOT_CREATOR, 1, &prefs, sizeof(SystemPreferencesType), true);

  if ((dbRef = PrefOpenPreferenceDB(true)) != NULL) {
    if ((num = DmNumResources(dbRef)) > 0) {
      debug(DEBUG_ERROR, PALMOS_MODULE, "there are %d items in legacy \"%s\" database", num, SAVED_PREFS);
    }
    DmCloseDatabase(dbRef);
  }

  if ((dbRef = PrefOpenPreferenceDB(false)) != NULL) {
    if ((num = DmNumResources(dbRef)) > 0) {
      debug(DEBUG_ERROR, PALMOS_MODULE, "there are %d items in legacy \"%s\" database", num, UNSAVED_PREFS);
    }
    DmCloseDatabase(dbRef);
  }

  mutex_destroy(mutex);
  mutex = NULL;

  return 0;
}

void PrefGetPreferences(SystemPreferencesPtr p) {
  if (mutex_lock(mutex) == 0) {
    MemMove(p, &prefs, sizeof(SystemPreferencesType));
    mutex_unlock(mutex);
  }
}

void PrefSetPreferences(SystemPreferencesPtr p) {
  if (mutex_lock(mutex) == 0) {
    MemMove(&prefs, p, sizeof(SystemPreferencesType));
    mutex_unlock(mutex);
  }
}

UInt32 PrefGetPreference(SystemPreferencesChoice choice) {
  UInt32 value = 0;

  if (mutex_lock(mutex) == 0) {
  switch (choice) {
    case prefVersion: value = prefs.version; break;
    case prefCountry: value = prefs.country; break;
    case prefDateFormat: value = prefs.dateFormat; break;
    case prefLongDateFormat: value = prefs.longDateFormat; break;
    case prefWeekStartDay: value = prefs.weekStartDay; break;
    case prefTimeFormat: value = prefs.timeFormat; break;
    case prefNumberFormat: value = prefs.numberFormat; break;
    case prefAutoOffDuration: value = prefs.autoOffDuration; break;
    case prefSysSoundLevelV20: value = prefs.sysSoundLevelV20; break;
    case prefGameSoundLevelV20: value = prefs.gameSoundLevelV20; break;
    case prefAlarmSoundLevelV20: value = prefs.alarmSoundLevelV20; break;
    case prefHidePrivateRecordsV33: value = 0; break; // XXX
    case prefDeviceLocked: value = prefs.deviceLocked; break;
    case prefLocalSyncRequiresPassword: value = prefs.localSyncRequiresPassword; break;
    case prefRemoteSyncRequiresPassword: value = prefs.remoteSyncRequiresPassword; break;
    case prefSysBatteryKind: value = prefs.sysBatteryKind; break;
    case prefAllowEasterEggs: value = 0; break; // XXX
    case prefMinutesWestOfGMT: value = prefs.minutesWestOfGMT; break;
    case prefDaylightSavings: value = prefs.daylightSavings; break;
    case prefRonamaticChar: value = prefs.ronamaticChar; break;
    case prefHard1CharAppCreator: value = prefs.hard1CharAppCreator; break;
    case prefHard2CharAppCreator: value = prefs.hard2CharAppCreator; break;
    case prefHard3CharAppCreator: value = prefs.hard3CharAppCreator; break;
    case prefHard4CharAppCreator: value = prefs.hard4CharAppCreator; break;
    case prefCalcCharAppCreator: value = prefs.calcCharAppCreator; break;
    case prefHardCradleCharAppCreator: value = prefs.hardCradleCharAppCreator; break;
    case prefLauncherAppCreator: value = prefs.launcherCharAppCreator; break;
    case prefSysPrefFlags: value = prefs.sysPrefFlags; break;
    case prefHardCradle2CharAppCreator: value = prefs.hardCradle2CharAppCreator; break;
    case prefAnimationLevel: value = prefs.animationLevel; break;
    case prefSysSoundVolume: value = prefs.sysSoundVolume; break;
    case prefGameSoundVolume: value = prefs.gameSoundVolume; break;
    case prefAlarmSoundVolume: value = prefs.alarmSoundVolume; break;
    case prefBeamReceive: value = prefs.beamReceive; break;
    case prefCalibrateDigitizerAtReset: value = prefs.calibrateDigitizerAtReset; break;
    case prefSystemKeyboardID: value = prefs.systemKeyboardID; break;
    case prefDefSerialPlugIn: value = prefs.defSerialPlugIn; break;
    case prefStayOnWhenPluggedIn: value = prefs.stayOnWhenPluggedIn; break;
    case prefStayLitWhenPluggedIn: value = prefs.stayLitWhenPluggedIn; break;
    case prefAntennaCharAppCreator: value = prefs.antennaCharAppCreator; break;
    case prefMeasurementSystem: value = prefs.measurementSystem; break;
    case prefShowPrivateRecords:
      if (!prefs.hideSecretRecords) {
        value = showPrivateRecords;
      } else if (prefs.maskPrivateRecords) {
        value = maskPrivateRecords;
      } else {
        value = hidePrivateRecords;
      }
      break;
    case prefAutoOffDurationSecs: value = prefs.autoOffDurationSecs; break;
    case prefTimeZone: value = prefs.timeZone; break;
    case prefDaylightSavingAdjustment: value = prefs.daylightSavingAdjustment; break;
    case prefAutoLockType: value = prefs.autoLockType; break;
    case prefAutoLockTime: value = prefs.autoLockTime; break;
    case prefAutoLockTimeFlag: value = prefs.autoLockTimeFlag; break;
    case prefLanguage: value = prefs.language; break;
    case prefLocale: value = 0; break; // XXX
    case prefTimeZoneCountry: value = prefs.timeZoneCountry; break;
    case prefAttentionFlags: value = prefs.attentionFlags; break;
    case prefDefaultAppCreator: value = prefs.defaultAppCreator; break;
    case prefDefFepPlugInCreator: value = prefs.defFepPlugInCreator; break;
    case prefColorThemeID: value = prefs.colorThemeID; break;
    case prefHandednessChoice: value = prefs.handedness; break;
    case prefHWRCreator: value = prefs.HWRCreator; break;
    default: value = 0; break;
  }
    mutex_unlock(mutex);
  }

  return value;
}

void PrefSetPreference(SystemPreferencesChoice choice, UInt32 value) {

  if (mutex_lock(mutex) == 0) {
  switch (choice) {
    case prefVersion: prefs.version = value; break;
    case prefCountry: prefs.country = value; break;
    case prefDateFormat: prefs.dateFormat = value; break;
    case prefLongDateFormat: prefs.longDateFormat = value; break;
    case prefWeekStartDay: prefs.weekStartDay = value; break;
    case prefTimeFormat: prefs.timeFormat = value; break;
    case prefNumberFormat: prefs.numberFormat = value; break;
    case prefAutoOffDuration: prefs.autoOffDuration = value; break;
    case prefSysSoundLevelV20: prefs.sysSoundLevelV20 = value; break;
    case prefGameSoundLevelV20: prefs.gameSoundLevelV20 = value; break;
    case prefAlarmSoundLevelV20: prefs.alarmSoundLevelV20 = value; break;
    case prefHidePrivateRecordsV33: break; // XXX
    case prefDeviceLocked: prefs.deviceLocked = value; break;
    case prefLocalSyncRequiresPassword: prefs.localSyncRequiresPassword = value; break;
    case prefRemoteSyncRequiresPassword: prefs.remoteSyncRequiresPassword = value; break;
    case prefSysBatteryKind: prefs.sysBatteryKind = value; break;
    case prefAllowEasterEggs: break; // XXX
    case prefMinutesWestOfGMT: prefs.minutesWestOfGMT = value; break;
    case prefDaylightSavings: prefs.daylightSavings = value; break;
    case prefRonamaticChar: prefs.ronamaticChar = value; break;
    case prefHard1CharAppCreator: prefs.hard1CharAppCreator = value; break;
    case prefHard2CharAppCreator: prefs.hard2CharAppCreator = value; break;
    case prefHard3CharAppCreator: prefs.hard3CharAppCreator = value; break;
    case prefHard4CharAppCreator: prefs.hard4CharAppCreator = value; break;
    case prefCalcCharAppCreator: prefs.calcCharAppCreator = value; break;
    case prefHardCradleCharAppCreator: prefs.hardCradleCharAppCreator = value; break;
    case prefLauncherAppCreator: prefs.launcherCharAppCreator = value; break;
    case prefSysPrefFlags: prefs.sysPrefFlags = value; break;
    case prefHardCradle2CharAppCreator: prefs.hardCradle2CharAppCreator = value; break;
    case prefAnimationLevel: prefs.animationLevel = value; break;
    case prefSysSoundVolume: prefs.sysSoundVolume = value; break;
    case prefGameSoundVolume: prefs.gameSoundVolume = value; break;
    case prefAlarmSoundVolume: prefs.alarmSoundVolume = value; break;
    case prefBeamReceive: prefs.beamReceive = value; break;
    case prefCalibrateDigitizerAtReset: prefs.calibrateDigitizerAtReset = value; break;
    case prefSystemKeyboardID: prefs.systemKeyboardID = value; break;
    case prefDefSerialPlugIn: prefs.defSerialPlugIn = value; break;
    case prefStayOnWhenPluggedIn: prefs.stayOnWhenPluggedIn = value; break;
    case prefStayLitWhenPluggedIn: prefs.stayLitWhenPluggedIn = value; break;
    case prefAntennaCharAppCreator: prefs.antennaCharAppCreator = value; break;
    case prefMeasurementSystem: prefs.measurementSystem = value; break;
    case prefShowPrivateRecords:
      switch (value) {
        case showPrivateRecords:
          prefs.hideSecretRecords = false;
          prefs.maskPrivateRecords = false;
          break;
        case maskPrivateRecords:
          prefs.hideSecretRecords = true;
          prefs.maskPrivateRecords = true;
          break;
        case hidePrivateRecords:
          prefs.hideSecretRecords = true;
          prefs.maskPrivateRecords = false;
          break;
      }
      break;
    case prefAutoOffDurationSecs: prefs.autoOffDurationSecs = value; break;
    case prefTimeZone: prefs.timeZone = value; break;
    case prefDaylightSavingAdjustment: prefs.daylightSavingAdjustment = value; break;
    case prefAutoLockType: prefs.autoLockType = value; break;
    case prefAutoLockTime: prefs.autoLockTime = value; break;
    case prefAutoLockTimeFlag: prefs.autoLockTimeFlag = value; break;
    case prefLanguage: prefs.language = value; break;
    case prefLocale: break; // XXX
    case prefTimeZoneCountry: prefs.timeZoneCountry = value; break;
    case prefAttentionFlags: prefs.attentionFlags = value; break;
    case prefDefaultAppCreator: prefs.defaultAppCreator = value; break;
    case prefDefFepPlugInCreator: prefs.defFepPlugInCreator = value; break;
    case prefColorThemeID: prefs.colorThemeID = value; break;
    case prefHandednessChoice: prefs.handedness = value; break;
    case prefHWRCreator: prefs.HWRCreator = value; break;
    default: break;
  }
    mutex_unlock(mutex);
  }
}

Int16 PrefGetAppPreferences(UInt32 creator, UInt16 id, void *prefs, UInt16 *prefsSize, Boolean saved) {
  Int16 version = noPreferenceFound;
  UInt16 size, *p;

  if (prefsSize) {
    size = pumpkin_get_preference(creator, id, NULL, 0, saved);
    if (size >= 2) {
      if ((p = xcalloc(1, size)) != NULL) {
        pumpkin_get_preference(creator, id, p, size, saved);
        if (*prefsSize == 0) {
          *prefsSize = size - 2;
        }
        if (prefs) {
          MemMove(prefs, &p[1], *prefsSize);
        }
        version = p[0];
        xfree(p);
      }
    }
  }

  return version;
}

void PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, const void *prefs, UInt16 prefsSize, Boolean saved) {
  UInt16 *p;

  if (prefs && prefsSize) {
    if ((p = xcalloc(1, 2 + prefsSize)) != NULL) {
      p[0] = version;
      MemMove(&p[1], prefs, prefsSize);
      pumpkin_set_preference(creator, id, p, 2 + prefsSize, saved);
      xfree(p);
    }
  }
}

Boolean PrefGetAppPreferencesV10(UInt32 type, Int16 version, void *prefs, UInt16 prefsSize) {
  Int16 vnum;

  pumpkin_set_v10();
  vnum = PrefGetAppPreferences(type, 1, prefs, &prefsSize, true);

  return vnum == version;
}

void PrefSetAppPreferencesV10(UInt32 creator, Int16 version, void *prefs, UInt16 prefsSize) {
  pumpkin_set_v10();
  PrefSetAppPreferences(creator, 1, version, prefs, prefsSize, true);
}

DmOpenRef PrefOpenPreferenceDB(Boolean saved) {
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenRef dbRef = NULL;

  if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, saved ? sysFileTSavedPreferences : sysFileTPreferences, sysFileCSystem, false, &cardNo, &dbID) == errNone) {
    dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadWrite);
  }

  return dbRef;
}

DmOpenRef PrefOpenPreferenceDBV10(void) {
  return PrefOpenPreferenceDB(false);
}

char *PrefCountryName(UInt32 i) {
  return i < NUM_COUNTRIES ? countries[i].name : NULL;
}

char *PrefLanguageName(UInt32 i) {
  return i < NUM_LANGUAGES ? languages[i].name : NULL;
}
