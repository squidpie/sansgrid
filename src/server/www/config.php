<?
/* ************************************************************************** */
/* START: USER DEFINABLE PARAMETERS ***************************************** */


// Server parameters
$SG['domain']			= "localhost";			// MySQL hostname
$SG['db_user']			= "sansgrid";			// MySQL user name
$SG['db_pass']			= "psu";				// MySQL user password

// Web parameters
$SG['project_url'] 		= "http://10.42.0.23";	// URL of project
$SG['project_title']	= "SansGrid";			// Displayed in browser window 


/* END: USER DEFINABLE PARAMETERS ******************************************* */
/* ************************************************************************** */
/* STOP! DO NOT MODIFY PARAMETERS BELOW THIS LINE! ************************** */

// Delimiters
$SG['kv_del'] = "α";						// Key-value delimiter
$SG['ff_del'] = "β";						// Parameter-to-parameter delimiter

// Maximum time, in seconds, duration between steps (within pipeline)
$SG['maxtime']['Peck']  	=  3 * 60;		// Peck - 5 minutes
$SG['maxtime']['Sing']  	=  3 * 60;		// Sing - 5 minutes
$SG['maxtime']['Mock']  	=  3 * 60;		// Mock - 5 minutes
$SG['maxtime']['Peacock1']  =  3 * 60;		// Peacock, more coming - 3 minutes
//$SG['maxtime']['Squawk1']   =  3 * 60;		// Squawk sent, no key - 3 minutes
//$SG['maxtime']['Squawk2']   =  3 * 60;		// Squawk -- awaiting sensor challenge.
//$SG['maxtime']['Squawk3']   =  3 * 60;		// Squawk -- responded to sensor's challenge.
$SG['maxtime']['Squawk1']   =  30;		// Squawk sent, no key - 3 minutes
$SG['maxtime']['Squawk2']   =  30;		// Squawk -- 
$SG['maxtime']['Squawk3']   =  360;		// Squawk -- need to comment
#$SG['maxtime']['FailedMate']= 10 * 60;		// A mating which never completed
										    // could leave residual data in places
											// other than the pipeline. 


$SG['skl'] = 128;			// Server key length.  Changing this value will almost
							// assuredly break things.  Just don't do it. 
							// Note: bytes * 2 = key length
//$SG['skl'] = 8;			
//$SG['skl'] = 4;			

$SG['debug'] = TRUE;		// Never, ever, ever set this to true!  And no matter
							// what happens, make sure you never feed it after
							// midnight!!!
?>
