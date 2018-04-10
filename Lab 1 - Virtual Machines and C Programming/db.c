#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "db.h"

// Maximum size of DB
static int maxSize = 0;

// Current number of records
static int numRecords = 0;

// The database
static TPhonebook *database = NULL;

// Initializes the phonebook for a maximum of maxRecords file.
// Pre: Phonebook is uninitialized, maxRecords contains maximum
// number of records in our phonebook.
// Post: Phonebook is initialized to maxRecords records, each of which is empty.
void initPhonebook(int maxRecords) {
	if(database != NULL)
		free(database);

	maxSize = maxRecords;
	numRecords = 0;

	database = (TPhonebook *) calloc(maxSize, sizeof(TPhonebook));
}

// Adds a new person into the phonebook.
// Pre: Phonebook has been initialized. name = Name of person,
// countryCode = 3 digit country code, phoneNumber = 7 digit phone number.
// Post: Person is inserted into phonebook if he isn't already there.
// The "result" parameter returns OK, MAX_REACHED or DUPLICATE.
void addPerson(char *name, char *countryCode, char *phoneNumber, int *result) {
	if(numRecords >= maxSize)
		*result = MAX_REACHED;
	else {
		if(findPerson(name) != NULL)
			*result = DUPLICATE;
		else {
			strncpy(database[numRecords].name, name, NAME_LENGTH);
			strncpy(database[numRecords].countryCode, countryCode, C_LENGTH);
			strncpy(database[numRecords].phoneNumber, phoneNumber, NUM_LENGTH);

			database->deleted = 0;
			database->index = numRecords++;
			*result = OK;
		}
	}
}

// Looks for a person in the phonebook. We do a full string match, and cannot do partial matches.
// Pre: Phonebook has been initialized. name = Name of person to search for.
// Post: Returns a pointer to the structure containing details of person if found, or NULL if not found.
TPhonebook *findPerson(char *name) {
	int i;
	int found = 0;
	TPhonebook *result = NULL;

	for(i = 0; i < numRecords && !found; i++) {
		found = (strcmp(name, database[i].name) == 0);

		if(found && !database[i].deleted)
			result = &database[i];
	}

	return result;
}

// Lists contents of phone book.
// Pre: Phonebook has been initialized.
// Post: Phonebook is listed on stdout.
void listPhonebook() {
	printf("\nPHONE LISTING\n");
	printf("===============\n\n");

	if(database != NULL) {
		int i;

		for(i = 0; i < numRecords; i++)
			if(!database[i].deleted)
				printf("%d: %s (%s) - (%s)\n", i + 1, database[i].name, database[i].countryCode, database[i].phoneNumber);
	}
	else
		printf("*** EMPTY ***\n\n");
}

// Deletes a person.
// Pre: Phonebook is initialized. name = Name of person to delete.
// Post: Person in "name" is removed if he exists and deletePerson returns OK. If he doesn't exist, function returns CANNOT_FIND.
int deletePerson(char *name) {
	TPhonebook *result = findPerson(name);

	if(result == NULL)
		return CANNOT_FIND;
	else {
		result->deleted = 1;
		return OK;
	}
}

// Saves phonebook
// Pre: Phonebook is initialized. filename = name of file to write phonebook to.
// Post: Returns OK if successful and data is written to filename, or SAVE_FAIL if an error occurs. Data is not guaranteed to be saved in this case.
int saveDB(char *filename) {
	FILE *fp = fopen(filename, "w");

	if(fp != NULL) {
		fprintf(fp, "%d %d\n", maxSize, numRecords);

		int i;
		for(i = 0; i < numRecords; i++)
			fprintf(fp, "%d\n%d\n%s\n%s\n%s\n", database[i].index, database[i].deleted, database[i].name, database[i].countryCode, database[i].phoneNumber);

		fclose(fp);
		return OK;
	}
	else
		return SAVE_FAIL;
}

// The fgets function reads everything in a line of text including the newline character. This is usually undesirable, so we define this function that checks if there's a newline character at the end of str and replaced it will a NULL terminator.

// This is a private function.
void stripNL(char *str) {
	int NLIdx = strlen(str) - 1;

	if(str[NLIdx] == '\r' || str[NLIdx] == '\n')
		str[NLIdx] = '\0';

}

// Loads phonebook.
// Pre: filename = name of file to load phonebook from. Phonebook does not need to be initialized.
// Post: If successful, phonebook is initialized and contents of filename are loaded, and function returns OK. If failure, function returns LOAD_FAIL and phonebook may be invalid.
int loadDB(char *filename) {
	FILE *fp = fopen(filename, "r");

	int dbSize, dbRecords;

	if(fp != NULL) {
		fscanf(fp, "%d %d\n", &dbSize, &dbRecords);

		printf("Read %d records max DB size is %d\n", dbRecords, dbSize);

		initPhonebook(dbSize);

		int i;
		for(i = 0; i < dbRecords; i++) {
			char buffer[128];

			fscanf(fp, "%d\n", &database[i].index);
			fscanf(fp, "%d\n", &database[i].deleted);

			fgets(buffer, NAME_LENGTH, fp);
			stripNL(buffer);
			printf("%d: Name: %s\n", i, buffer);
			strcpy(database[i].name, buffer);

			fgets(buffer, C_LENGTH, fp);
			stripNL(buffer);
			printf("%d: Country Code: %s\n", i, buffer);
			strcpy(database[i].countryCode, buffer);


			fgets(buffer, NUM_LENGTH, fp);
			stripNL(buffer);
			printf("%d: Phone Number: %s\n", i, buffer);
			strcpy(database[i].phoneNumber, buffer);

		}

		maxSize = dbSize;
		numRecords = dbRecords;
		return OK;
	}
	else
		return LOAD_FAIL;
}

// Resizes phonebook.
// Pre: Phonebook is previously initialized to a maximum size in number of records.
// numNewRecords = Maximum number of records is incremented by this number.
// Post: Maximum size of phonebook is incremented by numNewRecords.
void resizeDB(int numNewRecords) {
	maxSize += numNewRecords;

	database = (TPhonebook *) realloc(database, sizeof(TPhonebook) * maxSize);
}

// Gets size of phonebook.
// Pre: Phonebook is initialized.
// Post: nr = Number of non-empty records in phonebook, ms = Maximum size of phonebook in records.
void getDBSize(int *nr, int *ms) {
	*nr=numRecords;
	*ms=maxSize;
}

// Frees the phonebook.
// Pre: Phonebook was previously initialized by initPhonebook.
// Post: Phonebook is freed and no longer accessible.
void freePhonebook() {
	if(database != NULL)
		free(database);
}
