/*
    renamefiles - quick and easy rename files in 1 directory
    
    Copyright (C) 2015 Gien van den Enden - gien.van.den.enden@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 
*/  
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

// list of all filenames too be chaned
struct strFileRename {
  char                 *pOrgFilename ;
  char                 *pNewFilename ;
  struct strFileRename *pPrev        ;
  struct strFileRename *pNext        ;
};

static struct strFileRename *lstFileFirst = NULL ; // first entry in list of filenames
static struct strFileRename *lstFileLast  = NULL ; // last entry 

// settings
static int iSetNoHeader      = 0 ; // display header informatie 0 = yes, 1 = no
static int iSetNoFooter      = 0 ; // display footer informatie 0 = yes, 1 = no
static int iSetNoRename      = 0 ; // no rename files only display filesnames 1 = norename, 0 = rename
static int iSetNoFileNames   = 0 ; // diplay filenames , 0=yes, 1 = 0
static int iSetFailMatchInfo = 0 ; // display reason why filename is not matches (1=display)
static int iSetNoStarCheck   = 0 ; // check * in file selection

static char cSetFileNameSpit = '.' ; // character filename splitting

// version info
static char cVersion[] = "0.0.1"     ; // current version
static char cVerDate[] = "2015-08-04"; // yyyy-mm-dd

// global data for filepatternmatch()
static char cFileMatchStripped[ 2048 ] ; // used in filepatternmatch for the new filename
static char cReasonNoMatch[     4096 ] ; // reason why filename not matched

// renamefiles counters
static int iFilesRenamed = 0 ;
static int iFilesFailed  = 0 ;

//
// Exit code used:
//
// 0 = normal end                          
// 1 = filename in directory too long     (filepatternmatch)
// 2 = file selection given is too long   (filepatternmatch)
// 3 = rename pattern is too long         (filepatternmatch)
// 4 = unknown option 
// 5 = too many arguments, maximum 3 (exclusive options)
// 6 = too less arguments, minimum 2 (exclusive options)
// 7 = cannot open directory
// 8 = out of memory
// 9 = directory name is too long
// 10 = not * in the file selection, probably quotes are missing

// rename all the files in the list
void renameFiles( char *pDirectory )
{
   static char cFileFrom[ 8096 ];
   static char cFileTo[   8096 ];

   struct stat sb; // status file, needed too check if file exist
   
   struct strFileRename *pFileStr = lstFileFirst ;
   struct strFileRename *pCurrFile= NULL         ;
   
   int iLenDir ;
   int iResult ;

   // reset counters
   iFilesRenamed = 0 ;
   iFilesFailed  = 0 ;
   
   // check directory
   if ( pDirectory == NULL ) { 
      return ;
   }
   iLenDir = strlen( pDirectory );

   strcpy( cFileFrom, pDirectory );
   strcpy( cFileTo  , pDirectory );
   
   if ( iLenDir > 0 ) {
      if ( cFileFrom[ iLenDir - 1 ] != '/' ) {
         strcat( cFileFrom, "/" );
         strcat( cFileTo  , "/" );
         iLenDir++ ;
      }
   }
   // iLenDir is position where too add the filename
   
   // rename files
   while( pFileStr != NULL ) {
      pCurrFile = pFileStr ;
      pFileStr  = pFileStr->pNext ;
      
      strcpy( cFileFrom + iLenDir, pCurrFile->pOrgFilename );
      strcpy( cFileTo   + iLenDir, pCurrFile->pNewFilename );
      
      if ( iSetNoRename == 0 ) {
         // check same filename
         if ( strcmp( cFileFrom, cFileTo ) == 0 ) {
            iFilesRenamed++ ;
            if ( iSetNoFileNames == 0 ) {
               printf( "%-32s -> %s\n", pCurrFile->pOrgFilename, pCurrFile->pNewFilename );
            }
         }
         else if (stat( cFileTo, &sb) != -1 ) {
            // file exist 
            iFilesFailed++ ;
            fprintf( stderr, "%-32s <> %s FAILED; error: File already exist\n", pCurrFile->pOrgFilename, pCurrFile->pNewFilename );
         } else {   
            iResult = rename( cFileFrom, cFileTo );
            if ( iResult == -1 ) {
               // error
               iFilesFailed++ ;
               fprintf( stderr, "%-32s <> %s FAILED; error: %d, %s\n", pCurrFile->pOrgFilename, pCurrFile->pNewFilename, errno, strerror(errno) );
            } else {
               // ok
               iFilesRenamed++ ;
               if ( iSetNoFileNames == 0 ) {
                  printf( "%-32s -> %s\n", pCurrFile->pOrgFilename, pCurrFile->pNewFilename );
               }
            }
         }
      } else {
          // no rename only show
          if ( iSetNoFileNames == 0 ) {
             printf( "%-32s -> %s\n", pCurrFile->pOrgFilename, pCurrFile->pNewFilename );
          }   
      }
   }  
}

// add names too the list of filenames
int addNamesToList( char *pOrgName, char *pNewName ) 
{
   struct strFileRename *pNewStruct ;

   // create new entry
   pNewStruct = (struct strFileRename *) malloc( sizeof(struct strFileRename) ); 
   if ( pNewStruct == NULL ) {
      return 0; // out of memory
   }
   pNewStruct->pPrev        = lstFileLast ;
   pNewStruct->pNext        = NULL ;
   pNewStruct->pOrgFilename = strdup( pOrgName );
   pNewStruct->pNewFilename = strdup( pNewName );
  
   // out of memory check
   if ( pNewStruct->pOrgFilename == NULL ) { return 0; }
   if ( pNewStruct->pNewFilename == NULL ) { return 0; }
  
   // put entry in the list
   if ( lstFileLast != NULL ) {
      lstFileLast->pNext = pNewStruct ;
   }
   lstFileLast = pNewStruct ;
  
   // remember first entry
   if ( lstFileFirst == NULL ) {
      lstFileFirst = pNewStruct ;
   }
   return 1; // all ok
}

// free the list
void freeList()
{
   // int iCount = 0 ;
   while( lstFileFirst != NULL ) {
      lstFileLast  = lstFileFirst ;
      lstFileFirst = lstFileFirst->pNext ;
      
      // iCount++ ;
      
      free( lstFileLast->pOrgFilename );
      free( lstFileLast->pNewFilename );
      free( lstFileLast );
   }
   lstFileLast = NULL ;
   // printf( "Freed entrys: %d\n\n", iCount );
}

// print help information
void printHelp()
{
   printf ( "\nUsage:\n renamefiles [options] [directory] file-selection rename-pattern\n\n" );

   printf ( "Options:\n" );
   printf ( " --help             show help and exit\n" );
   printf ( " --version          show version and exit\n" );
   printf ( " --norename         show only filenames too rename\n" );
   printf ( " --noheader         display no header information\n" );
   printf ( " --nofooter         display no footer information\n" ); 
   printf ( " --nofilenames      display no filenames\n" ); 
   printf ( " --nooutput         display no information only errors\n" );
   printf ( " --nostarcheck      no check for a wildchar (*) in the file selection\n" );
   printf ( " --failmatchinfo    display filenames witch do not match and the reason\n" );
   printf ( " --nonamesplit      Use the filename as 1 string\n" );
   printf ( "\n" );
   printf ( "The only wildcard in the file-selection and rename-pattern is a *\n" );
   printf ( "Quotes the parameters\n" );
   printf ( "\n" );
   printf ( "See how to use file-selection, rename-pattern and directory arguments from renamefiles manual.\n" );
   printf ( "\n" );
}

// print version information and exit
void printVersion()
{
   printf( "\n" );
   printf( "renamefiles\nversion: %s\n", cVersion );
   printf( "\n" );
  
   printf( "Copyright (C) 2015 Gien van den Enden\n");
   printf( "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
   printf( "This is free software: you are free to change and redistribute it.\n");
   printf( "There is NO WARRANTY, to the extent permitted by law.\n");
   printf( "\n" );

   exit(0);
}

// 
// Match filename (pntFilename) with file selection (pntPattern)
// and fill the rename filename in cFileMatchStripped according 
// too the rename pattern (pntMatchName)
// 
// return 
// 0 = no match
// 1 = filename match with file selection, cFileMatchStripped is filled with the rename filename
//
int filepatternmatch( char *pntFilename, char *pntPattern, char *pntMatchName )
{
   // reset buffers
   memset( cFileMatchStripped, 0, sizeof( cFileMatchStripped ) );
   memset( cReasonNoMatch    , 0, sizeof( cReasonNoMatch     ) );
  
   // there must be something too match
   if ( pntFilename == NULL ) { return 0; }
   if ( pntPattern  == NULL ) { return 0; }
    
   //
   // the pattern match is for filename parts
   // parts are seperated by . (point)
   // a wildchar is only valid for a filename part
   // the only wildchar is a *
   //
    
   // filename = normale filename in directory
   // pattern = filename with *
   //           * = 0, 1 of more characters
   //
   int iMatch         = 1 ; // assume it matches
    
   int iLenfilename    = strlen( pntFilename  );
   int iLenPattern     = strlen( pntPattern   );
   int iLenMatch       = strlen( pntMatchName );
   
   int iCntFile        =  0 ; // current position in the filename
   int iPosPattern     =  0 ; // current position in the pattern
   int iPosMatch       =  0 ; // current position in match pattern
   int iPosStripped    =  0 ; // current position in cFileMatchStripped = new filename
    
   int iFileParts      =  0 ; // number of parts in the filename
   int iPattParts      =  0 ; // number of parts in the pattern

   int iPartStartFile  =  0 ; // start in the filename of the current part
   int iPartStartPatt  =  0 ; // start in the pattern of the current part
   int iPartStartMatch =  0 ; // start in the match of the current part
   int iPartStartStrip =  0 ; // start in the match of the stripped file (new name)
   
   int iPosStarFile    = -1 ; // position last star in filename
   int iPosStarPatt    = -1 ; // position last star in pattern 
   int iPosStarMatch   = -1 ; // position last star in match
   int iPosStarstrip   = -1 ; // position last star in strip
    
   // some hard coded checks...
   if ( iLenfilename > 1024 ) {
      fprintf( stderr, "Filename length greater then 1024 not supported, program terminated\n" );
      exit( 1 );
   }
   if ( iLenPattern > 1024 ) { 
      fprintf( stderr, "File selection length greater then 1024 not supported, program terminated\n" );
      exit( 2 );
   }
   if ( iLenMatch > 1024 ) {
      fprintf( stderr, "Change selection length greater then 1024 not supported, program terminated\n" );
      exit( 3 );
   }
    
   // check if the number of parts in the filename
   // and patterns is equal
   // if not no match
   for( iCntFile = 0; iCntFile < iLenfilename; iCntFile++ ) {
      if ( cSetFileNameSpit != '\0' && pntFilename[ iCntFile ] == cSetFileNameSpit ) {
         iFileParts++ ;
      }
   }
   for( iCntFile = 0; iCntFile < iLenPattern; iCntFile++ ) {
      if ( cSetFileNameSpit != '\0' && pntPattern[ iCntFile ] == cSetFileNameSpit ) {
         iPattParts++ ;
      }
   }
   if ( iFileParts != iPattParts ) {
      // printf( "Parts does not match, iFileParts:%d iPattParts:%d\n", iFileParts, iPattParts );
      sprintf( cReasonNoMatch, "File parts does not match, number of file parts: %d, number of rename parts: %d", iFileParts, iPattParts );
      return 0 ;
   }
    
   // copy the fixed part of the pntMatchName into the new filename
   // that's all until a . or a *
   iPosMatch = 0 ;
   while( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] != cSetFileNameSpit && pntMatchName[ iPosMatch ] != '*' ) {
      cFileMatchStripped[ iPosStripped ] = pntMatchName[ iPosMatch ] ;
      iPosStripped++ ;
      iPosMatch++    ;
   }
    
   // walk filename for each character
   // check each chacter with the pattern
   for( iCntFile = 0; iCntFile < iLenfilename; iCntFile++ ) {
      // check the end of the pattern
      if ( iPosPattern >= iLenPattern ) {
         iMatch = 0 ;
         sprintf( cReasonNoMatch, "Filename is longer (%d) then the file-selection", iLenfilename );
         break ;
      }
      
      if ( cSetFileNameSpit != '\0' && pntFilename[ iCntFile ] == cSetFileNameSpit ) {
         int iFoundStartPattern = 0 ;
         // printf( "Part check\n" );

         // check end of part can be reach with last star
         if ( pntPattern[ iPosPattern ] != cSetFileNameSpit && pntPattern[ iPosPattern ] != '*' && iPosStarPatt >= 0 ) {

            // there is a start set pattern counter back
            iPosPattern = iPosStarPatt ;
            
            // reset star back
            if ( iPosStarMatch >= 0 ) {
               iPosMatch = iPosStarMatch ;
            }   
            
            iCntFile = iPosStarFile ; // for make it 1 higher
            
            if ( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] == '*' ) {
               cFileMatchStripped[ iPosStripped ] = pntFilename[ iCntFile ] ;
               iPosStripped++ ;
            }
            continue ;
         }
         
         // copy all match fixed charaxters until end of part
         while( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] != cSetFileNameSpit ) {
           if ( pntMatchName[ iPosMatch ] != '*' ) {
              cFileMatchStripped[ iPosStripped ] = pntMatchName[ iPosMatch ] ;
              iPosStripped++ ;
           } else {
              iFoundStartPattern = 1 ;
           }
           iPosMatch++ ;
         }
         if( iPosMatch < iLenMatch && cSetFileNameSpit != '\0' && pntMatchName[ iPosMatch ] == cSetFileNameSpit ) {
            cFileMatchStripped[ iPosStripped ] = pntMatchName[ iPosMatch ] ;
            iPosStripped++ ;
            iPosMatch++ ;
         }

         // part check
         if( pntPattern[ iPosPattern ] != cSetFileNameSpit ) {
            // if not a wildcard then end of match
            if( pntPattern[ iPosPattern ] != '*' ) {
                sprintf( cReasonNoMatch, "End of filename part reached but selection not, file pos: %d, selection pos: %d, char %c", iCntFile, iPosPattern, pntPattern[ iPosPattern ] );
                iMatch = 0;
                break ;
            }

            
            iPosPattern++ ;
            // part expected, not found, and of match
            if( pntPattern[ iPosPattern ] != cSetFileNameSpit ) {
                sprintf( cReasonNoMatch, "End of filename part reached but selection not, file pos: %d, selection pos: %d, char %c", iCntFile, iPosPattern, pntPattern[ iPosPattern ] );
                iMatch = 0 ;
                break ;
            }
         } else {
            // printf( "speciale case\n" );
            // printf( "iCntFile       : %d\n", iCntFile        );
            // printf( "iPosStarFile   : %d\n", iPosStarFile    );
            // printf( "iPartStartMatch: %d\n", iPartStartMatch );
            // printf( "iPosMatch      : %d, char %c\n", iPosMatch, pntMatchName[ iPosMatch ]       );
            // printf( "iFoundStartPattern: %d\n", iFoundStartPattern );
            
            // printf( "iPosStarPatt   : %d\n", iPosStarPatt    );
            // printf( "iPosPattern    : %d\n", iPosPattern     );
            // printf( "iPartStartPatt : %d\n", iPartStartPatt  );
            // printf( "iPosStripped   : %d\n", iPosStripped    );
            // printf( "iPartStartStrip: %d\n", iPartStartStrip );
            // printf( "cFileMatchStripped: %s\n", cFileMatchStripped );
            
            // speciale case
            // if there is no wildchar in the file selection part
            // and de match pattern only contains a wildcard 
            // then copy all the filename part
            if ( ( iPosStarFile              == -1              ) &&  /* no wildchar found in selection */
                 ( iPosStripped - 1          == iPartStartStrip ) &&  /* nothing copied, point is already copied */
                 ( iFoundStartPattern        == 1               )     /* wildchar in return pattern */
                 //( pntMatchName[ iPosMatch + 1 ] == '.' || pntMatchName[ iPosMatch + 1 ] == '\0') 
               ) {
            
               iPosStripped-- ; // point was already copied
               while( iPartStartFile <= iCntFile ) {
                  cFileMatchStripped[ iPosStripped ] = pntFilename[ iPartStartFile ] ;
                  iPosStripped++ ;
                  iPartStartFile++;
               }
            }
            
         }
         // reset star position
         iPosStarFile   = -1 ; // position last star in filename
         iPosStarPatt   = -1 ; // position last star in pattern 
         iPosStarMatch  = -1 ; 
         iPosStarstrip  = -1 ;

         iPosPattern++ ;
         // printf( "Part check after: %d\n", iPosPattern );

         // new start a the part's
         iPartStartFile = iCntFile + 1 ;
         iPartStartPatt = iPosPattern  ;
         iPartStartMatch= iPosMatch    ;
         iPartStartStrip= iPosStripped ;

         // printf( "Start new part\n" );
         // printf( "iPartStartFile : %d\n", iPartStartFile );         
         // printf( "iPartStartMatch: %d\n", iPartStartMatch );
         // printf( "iPosMatch: %d\n"      , iPosMatch );

         // copy all the fixed chars at the beginnen of the pattern part
         while( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] != cSetFileNameSpit && pntMatchName[ iPosMatch ] != '*' ) {
            cFileMatchStripped[ iPosStripped ] = pntMatchName[ iPosMatch ] ;
            iPosStripped++ ;
            iPosMatch++    ;
         }
         
      } else if ( pntFilename[ iCntFile ] == pntPattern[ iPosPattern ] ) {
         // check if character match
         iPosPattern++ ;
         
      } else if( pntPattern[ iPosPattern ] == '*' ) {
         // check the character after the pattern
         // if that matchs go to the next character
         if ( iPosPattern < iLenPattern ) {
            if ( pntFilename[ iCntFile ] == pntPattern[ iPosPattern + 1 ] ) {
               // problem -> ffanaam f*am
               // set star position
               iPosStarFile  = iCntFile    ; // position last star in filename
               iPosStarPatt  = iPosPattern ; // position last star in pattern 
               iPosStarMatch = -1          ;
               iPosStarstrip = -1          ;
               
               iPosPattern++ ;
               iPosPattern++ ;
               
               // copy all character with matching *
               if ( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] == '*' ) {
                  iPosStarMatch = iPosMatch    ; // remember star in match
                  iPosStarstrip = iPosStripped ;
                  iPosMatch++ ; // end of wildchar, get next in match pattern
               }
               // copy fixed part in match pattern
               while( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] != cSetFileNameSpit && pntMatchName[ iPosMatch ] != '*' ) {
                  cFileMatchStripped[ iPosStripped ] = pntMatchName[ iPosMatch ] ;
                  iPosStripped++ ;
                  iPosMatch++ ;
               }
            } else {
               // copy all character with matching *
               if ( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] == '*' ) {
                  cFileMatchStripped[ iPosStripped ] = pntFilename[ iCntFile ] ;
                  iPosStripped++ ;
               }
            }
         } else {
            // copy all character with matching *
            if ( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] == '*' ) {
                cFileMatchStripped[ iPosStripped ] = pntFilename[ iCntFile ] ;
                iPosStripped++ ;
            }
         }
      } else {
         // it does not match
         if ( iPosStarPatt >= 0 ) {
            // there is a start set pattern counter back
            iPosPattern = iPosStarPatt ;
            
            // reset star back
            if ( iPosStarMatch >= 0 ) {
               iPosMatch = iPosStarMatch ;
            }   
            // if fixed in is copy set position back
            if ( iPosStarstrip >= 0 ) {
               iPosStripped = iPosStarstrip ;
            }
            iCntFile = iPosStarFile ; // the for make it 1 bigger
            
            if ( iPosMatch < iLenMatch && pntMatchName[ iPosMatch ] == '*' ) {
               cFileMatchStripped[ iPosStripped ] = pntFilename[ iCntFile ] ;
               iPosStripped++ ;
            }
         } else {
            sprintf( cReasonNoMatch, "File not match with selection, file pos: %d, char: %c selection pos: %d, char: %c", iCntFile, pntFilename[ iCntFile ], iPosPattern, pntPattern[ iPosPattern ] );
            iMatch = 0 ;
            break;
         }
      }
   }
    
   if ( iMatch == 1 ) {
      // all character must be used of the pattern
      if ( iPosPattern != iLenPattern ) {
         iMatch = 0 ;
         // if the last character of the pattern is a * then it is valid
         if ( iPosPattern == iLenPattern - 1 ) {
            if ( pntPattern[ iPosPattern ] == '*' ) {
               iMatch = 1 ;
            }
         }
         if ( iMatch == 0 ) {
            sprintf( cReasonNoMatch, "End of filename reached but selection not, selection pos: %d, char %c", iPosPattern, pntPattern[ iPosPattern ] );
         }
      }
   }
   if ( iMatch == 1 ) {
      // speciale case
      // if there is no wildchar in the file selection part
      // and de pattern only contains a wildcard 
      // then copy all the filename part
      if ( ( iPosStarFile              == -1              ) &&  /* no wildchar found in selection */
           ( iPosMatch                 == iPartStartMatch ) &&  /* nothing copied */
           ( pntPattern[ iPosPattern ] != '*'             ) &&  /* current patern char is not a wildchar */
           ( pntMatchName[ iPosMatch + 1 ] == cSetFileNameSpit || pntMatchName[ iPosMatch + 1 ] == '\0') ) {
         
         // printf( "speciale case end\n" );
         // printf( "iPosStarFile   : %d\n", iPosStarFile    );
         // printf( "iPartStartMatch: %d\n", iPartStartMatch );
         // printf( "iPosMatch      : %d\n", iPosMatch       );
         // printf( "iPosStarPatt   : %d\n", iPosStarPatt    );
         // printf( "iPosPattern    : %d\n", iPosPattern     );
         // printf( "iPartStartPatt : %d\n", iPartStartPatt  );

         while( iPartStartFile < iCntFile ) {
            cFileMatchStripped[ iPosStripped ] = pntFilename[ iPartStartFile ] ;
            iPosStripped++ ;
            iPartStartFile++;
         }
      }
      
      // if there are more characters in match 
      while( iPosMatch < iLenMatch ) {
         if ( pntMatchName[ iPosMatch ] != '*' ) {
            cFileMatchStripped[ iPosStripped ] = pntMatchName[ iPosMatch ] ;
            iPosStripped++ ;
         }
         iPosMatch++ ;
      }
   }
   return iMatch;
}

//
// Program start point
// 
int main( int argc, char *argv[] ) {
  
   struct dirent *pnt_dir_entry   ;
   DIR           *pnt_directory   ;
   int            iFilesDir       ; // number of files in directory found
   int            iFilesMatch     ; // number of files match selection

   int            iArgCount       ;
   char          *pArguments[ 3 ] ; // max 3 arguments
   int            iCntArgv        ;
   
   char          *pDirectory      ; // pointer too directory name
   char          *pFileSelection  ; // pointer too file selection
   char          *pRenamePattern  ; // pointer too rename pattern
   
   //
   // parse commandline parameters
   //
   iArgCount   = -1 ; // number of arguments found
   for( iCntArgv = 1; iCntArgv < argc; iCntArgv++ ) {
     if (        strcmp( argv[ iCntArgv ], "--version"  ) == 0 ) {
         printVersion(); // this function exit program
         
     } else if ( strcmp( argv[ iCntArgv ], "--noheader" ) == 0 ) {
         iSetNoHeader = 1 ;
         
     } else if ( strcmp( argv[ iCntArgv ], "--nofooter" ) == 0 ) {
         iSetNoFooter = 1 ;
         
     } else if ( strcmp( argv[ iCntArgv ], "--nofilenames" ) == 0 ) {
         iSetNoFileNames = 1 ;
         
     } else if ( strcmp( argv[ iCntArgv ], "--nostarcheck" ) == 0 ) {
         iSetNoStarCheck = 1 ;

     } else if ( strcmp( argv[ iCntArgv ], "--help"     ) == 0 ) {
         printHelp(); // display help 
         exit(0);
         
     } else if ( strcmp( argv[ iCntArgv ], "--norename" ) == 0 ) {
         iSetNoRename = 1 ;
         
     } else if ( strcmp( argv[ iCntArgv ], "--nooutput" ) == 0 ) {
         iSetNoFileNames = 1 ;
         iSetNoHeader    = 1 ;
         iSetNoFooter    = 1 ;
         
     } else if ( strcmp( argv[ iCntArgv ], "--failmatchinfo" ) == 0 ) {
         iSetFailMatchInfo = 1 ;

     } else if ( strcmp( argv[ iCntArgv ], "--nonamesplit" ) == 0 ) {
         cSetFileNameSpit = '\0' ;
         
     } else if ( strncmp( argv[ iCntArgv ], "--", 2  ) == 0 ) {
         fprintf( stderr, "Unknown option: %s\n", argv[ iCntArgv ] );
         printHelp(); // display help 
         exit( 4 );
         
     } else {
       if ( iArgCount >= 2 ) {
         fprintf( stderr, "Too many arguments, maxium is 3, exclusive options (quotes the parameters?)\n" );
         printHelp(); // display help 
         exit( 5 );
       } 
       iArgCount++ ;
       pArguments[ iArgCount ] = argv[ iCntArgv ] ;
     }
   }
   
   // determined the directory, file selecton and rename pattern
   if ( iArgCount < 1 ) {
      fprintf( stderr, "Too few arguments, needed at least 2 exclusive options\n" );
      printHelp(); // display help 
      exit(6);
   }
   
   if ( iArgCount >= 2 ) {
      pDirectory     = pArguments[ 0 ] ;
      pFileSelection = pArguments[ 1 ] ;
      pRenamePattern = pArguments[ 2 ] ;
   } else {
      pDirectory     = "."             ; // default directory
      pFileSelection = pArguments[ 0 ] ;
      pRenamePattern = pArguments[ 1 ] ;
   }

   // check if there is a wildchar in the fileselection
   // if not most likely the quotes are missing
   // the shell do automatich expansion of wildcar on the commandline
   if ( iSetNoStarCheck == 0 ) {
      if ( strchr( pFileSelection, '*' ) == NULL ) {
         fprintf( stderr, "Not one wildchar (*) found in the file selection, probebly the quotes are missing.\n" );
         exit(10);        
      }
   }
   
   //
   // read filenames from directory
   //
   if ( strlen( pDirectory ) > 6000 ) {
      fprintf( stderr, "Directory name is longer then 6000 characters\n" ); 
      exit(9);
   }
  
   pnt_directory = opendir( pDirectory );
   if ( pnt_directory == NULL ) {
      fprintf( stderr, "Cannot open directory %s\n", argv[1] ); 
      exit(7);
   }

   // display header information
   if ( iSetNoHeader == 0 ) {
      printf( "\n" );
      printf( "Directory         : %s\n" , pDirectory     );
      printf( "File selection    : %s\n" , pFileSelection );
      printf( "Rename pattern    : %s\n" , pRenamePattern );
      printf( "-------------------\n" );
   }
  
   // walk all the files in the directory
   iFilesDir   = 0 ;
   iFilesMatch = 0 ;
   
   while ( ( pnt_dir_entry = readdir( pnt_directory ) ) != NULL) {
      // only real files
      if ( pnt_dir_entry->d_type == DT_REG ) {
        
         iFilesDir++ ;
         
         if ( filepatternmatch(  pnt_dir_entry->d_name, pFileSelection, pRenamePattern ) != 0 ) {
            iFilesMatch++;
            if ( addNamesToList( pnt_dir_entry->d_name, cFileMatchStripped ) == 0 ) {
               fprintf( stderr, "Out of memory, number of files stored in list: %d\n", iFilesMatch );
               exit(8);                 
            }
         } else {
            if ( iSetFailMatchInfo == 1 ) {
               printf( "%-32s NOMATCH: %s\n", pnt_dir_entry->d_name, cReasonNoMatch );
            }
         }
      }
   }
   closedir( pnt_directory );

   // rename all the files in the linked list
   renameFiles( pDirectory );

   
   // display footer
   if ( iSetNoFooter == 0 ) {
      printf( "-------------------\n" );
      printf( "Files in directory: %6d\n" , iFilesDir     );
      printf( "Files match       : %6d\n" , iFilesMatch   );
      printf( "Files renamed     : %6d\n" , iFilesRenamed );
      printf( "Files failed      : %6d\n" , iFilesFailed  );
      
      printf( "\n" );
   }
 
   // free the created filename list
   freeList();
   
   return 0; // all is done
}