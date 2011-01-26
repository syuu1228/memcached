/* 
 * MMCSTORAGE - Database Database Linker
 *
 * AUTHOR
 *   Toru Maesaka <tmaesaka@users.sourceforge.net>
 */

typedef struct {
	int unused;
} MMCSTORAGE;


/* Creates an object of the linked storage engine. 
   Returns the MMCSTORAGE object on success and NULL on failure. */
MMCSTORAGE *mmcstorage_new(void);

/* Frees the MMCSTORAGE object 

   stor  =  MMCSTORAGE object
*/
void mmcstorage_destruct(MMCSTORAGE *stor);

/* Opens a file associated to the linked storage engine 
   
   stor   = MMCSTORAGE object
   cmd    = An arbitrary character string that can be used to represent a 
            command. Currently the format of the command is module 
            specific but I am planning on creating a DBI-like specification
            for this.

   Returns 1 on success and -1 on failure. */
int mmcstorage_open(MMCSTORAGE *stor, const char *cmd);


/* Closes the linked storage engine. 

   stor = MMCSTORAGE object

   Returns 1 on success and -1 on failure. */
int mmcstorage_close(MMCSTORAGE *stor);


/* Configure/Tune the paramaters of the linked storage engine. 
   This function is optional.

   The specification of this function depends on the module.

   stor = MMCSTORAGE object
   num  = An arbitrary integer that can be used.
   opts = An arbitrary character string that can be used.

   Returns 1 on success and -1 on failure. */
int mmcstorage_conf(MMCSTORAGE *stor, const int num, const char *opts);

void *mmcstorage_alloc(MMCSTORAGE *stor, const void *kbuf, const int ksiz, 
					   const int vsiz);

/* Stores a given record into the storge engine.

   stor  =  MMCSTORAGE object
   kbuf  =  Pointer to the key region
   ksiz  =  Size of the key
   vbuf  =  Pointer to the value region
   vsiz  =  Size of the value

   Returns 1 on success and -1 on failure. */
int mmcstorage_put(MMCSTORAGE *stor, const void *kbuf, const int ksiz, 
                   const void *vbuf, const int vsiz);


/* Retrieves a value mapped to a given key. 

   stor  =  MMCSTORAGE object
   kbuf  =  Pointer to the key region
   ksiz  =  Size of the key
   vsizp =  Pointer to the variable for assigning the size
            of the fetched value. 

   Returns the pointer to the value on success, and  NULL on failure. */
void *mmcstorage_get(MMCSTORAGE *stor, const void *kbuf, const int ksiz,
                     int *vsizp);


int mmcstorage_free(MMCSTORAGE *stor, const void *vbuf, const int vsiz);

/* Deletes a record mapped to the given key from the storage.

   stor  =  MMCSTORAGE object
   kbuf  =  Pointer to the key region
   ksiz  =  Size of the key

   Return 1 on success and -1 on failure. */
int mmcstorage_del(MMCSTORAGE *stor, const void *kbuf, const int ksiz);

/* Deletes every record of the linked storage object.
   
   stor  =  MMCSTORAGE object
   opts  =  An arbitrary character string that can be used as an option.

    Returns 1 on success and -1 on failure. */
int mmcstorage_flush(MMCSTORAGE *stor, const char *opts);

