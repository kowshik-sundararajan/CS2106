#ifndef PHONEBOOK

#define PHONEBOOK

#define NAME_LENGTH    64
#define C_LENGTH       4
#define NUM_LENGTH     10


typedef struct
{
	unsigned index;
	int deleted;
	char name[NAME_LENGTH];
	char countryCode[C_LENGTH];
	char phoneNumber[NUM_LENGTH];
} TPhonebook;

enum {
	OK = 0,
	MAX_REACHED = 1,
	DUPLICATE = 2,
	CANNOT_FIND = 3,
	SAVE_FAIL = 4,
	LOAD_FAIL = 5
};

// Initializes the phonebook for a maximum of maxRecords file.
// Pre: Phonebook is uninitialized, maxRecords contains maximum
// number of records in our phonebook
// Post: Phonebook is initialized to maxRecords records, each of which is empty.
void initPhonebook(int maxRecords);

// Frees the phonebook.
// Pre: Phonebook was previously initialized by initPhonebook
// Post: Phonebook is freed and no longer accessible.
void freePhonebook();

// Adds a new person into the phonebook.
// Pre: Phonebook has been initialized. name = Name of person, countryCode = 3 digit country code, phoneNumber = 7 digit phone number
// Post: Person is inserted into phonebook if he isn't already there. The "result" parameter returns OK, MAX_REACHED or DUPLICATE
void addPerson(char *name, char *countryCode, char *phoneNumber, int *result);

// Looks for a person in the phonebook. We do a full string match, and cannot do partial matches.
// Pre: Phonebook has been initialized. name = Name of the person to search for.
// Post: Returns a pointer to the structure containing details of person if found, or NULL if not found.
TPhonebook *findPerson(char *name);

// Lists content of phonebook.
// Pre: Phonebook has been initialized.
// Post: Phonebook is listed on stdout.
void listPhonebook();

// Deletes a person from the phonebook.
// Pre: Phonebook is initialized. name = name of person to delete. Full string matches only.
// Post: Person in "name" is removed if he exists and deletePerson returns OK. If he doesn't exist, function returns CANNOT_FIND.
int deletePerson(char *name);

// Save phonebook.
// Pre: Phonebook is initialized. filename = name of file to write phonebook to.
// Post: Returns OK if successful and data is written to filename, or SAVE_FAIL if an error occurs. Data is guaranteed to be saved in this case.
int saveDB(char *filename);

// Load phonebook.
// Pre: filename = name of the file to load phonebook from. Phonebook does not have to be initialized.
// Post: If successful, phonebook is initialized and contents of filename are loaded, and function returns OK. If failure, function returns LOAD_FAIL and phonebook may be invalid.
int loadDB(char *filename);

// Resize phonebook.
// Pre: Phonebook is previously initialized to a maximum size in number of records.
// numNewRecords = Maximum number of records is incremented by this number.
// Post: Maximum size of phonebook is incremented by numNewRecords.
void resizeDB(int numNewRecords);

// Get size of phonebook
// Pre: Phonebook is initialized.
// Post: nr = Number of non-empty records in phonebook, ms = Maximum size of phonebook in records.
void getDBSize(int *nr, int*ms);

#endif
