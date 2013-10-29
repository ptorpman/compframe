//=============================================================================
// uuidtool 
//
//            Tool to create, print and manipulate OSF/DCE UUIDs
//
//            (C) 1998 Jim Doyle
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//=============================================================================

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>


#include "uuid.hh"

void mk_dce_idl_hdr();
void mk_c_initializer();
void mk_com_initializer();
void test_ms_guid();
void test_dce_uuid();
void test_lt();
void test_gt();
void test_eq();

//===========================================================================
// usage():
//
//    Print information on using this tool.
//
//===========================================================================

void usage()
{
  printf(" uuid: Create,Print and Use OSF/DCE Universal Unique Identifiers\n");
  printf("           (C)1998 Jim Doyle, Boston University, <jrd@bu.edu>");
  printf("\n");
  printf("\n\t\t\t General Functions\n\n");
  printf(" -i\t\tProduce a DCE RPC IDL header\n");
  printf(" -s\t\tGenerates a UUID string as an initialized C structure\n");
  printf(" -c\t\tGenerates a Microsoft COM GUID as an initialized C struct\n");
  printf(" -n [num]\tGenerate 'num' new UUIDs\n");
  printf(" -h, -?\t\tPrint this help\n");
  printf("\n\t\t\t Comparison Functions\n\n");
  printf(" -e [uuid1],[uuid2]\tExit status of 0 if uuid1 == uuid2\n");
  printf(" -l [uuid1],[uuid2]\tExit status of 0 if uuid1 < uuid2\n");
  printf(" -g [uuid1],[uuid2]\tExit status of 0 if uuid1 > uuid2\n");
  printf(" -t [uuid]\t\tExit status of 0 if UUID is valid and not NIL UUID\n");
  printf(" -d [uuid]\t\tExit status of 0 if given UUID is an OSF/DCE UUID\n");
  printf(" -m [uuid]\t\tExit status of 0 if given UUID is a Microsoft GUID\n");
  printf("\n");
  printf(" default action: create and print one new UUID.\n");
  printf("\n");
  printf(" Note:  uuid1 > uuid2 if uuid1 is later in time than uuid2\n");
  printf("        uuid1 < uuid2 if uuid2 was create after uuid1 \n");
  printf("\n");

  exit(1);

}

//
// Globals and what not
//

static int num_uuids = 1;
char * uuid_args;
uuid uuid1(nil_uuid);
uuid uuid2(nil_uuid);
bool opt_idl_hdrgen, opt_c_initializer, opt_com_initializer;
bool opt_equals, opt_lessthan, opt_greaterthan, opt_test;
bool opt_testdceuuid, opt_testmsguid;

//===========================================================================
// main()
//===========================================================================

main(int argc, char * argv[])
{

  extern char * optarg;
  extern int optind, opterr, optopt;
  int j;

  //
  // Pick apart the command line args
  //

  while ((j = getopt(argc, argv, "visch?n:e:l:g:t:d:m:")) != EOF)
    {
      uuid_args = 0;
      switch(j)
	{
	case 'i':
	  opt_idl_hdrgen = true;
	  break;
	case 's':
	  opt_c_initializer = true;
	  break;
	case 'c':
	  opt_com_initializer = true;
	  break;
	case 'h':
	case '?':
	case 'v':
	  usage();
	  break;
	case 'n':
	  num_uuids = atoi(optarg);
	  break;
	case 'e':
	  opt_equals = true;
	  uuid_args = optarg;
	  break;
	case 'l':
	  opt_lessthan = true;
	  uuid_args = optarg;
	  break;
	case 'g':
	  opt_greaterthan = true;
	  uuid_args = optarg;
	  break;
	case 't':
	  opt_test = true;
	  uuid_args = optarg;
	  break;
	case 'd':
	  opt_testdceuuid = true;
	  uuid_args = optarg;
	  break;
	case 'm':
	  opt_testmsguid = true;
	  uuid_args = optarg;
	  break;
	default:
	  usage();
	}
    }



  //
  // For options that take a single UUID, go get it
  //

  if (opt_test || opt_testmsguid || opt_testdceuuid)
    {
      uuid1 = uuid(uuid_args);
      if (uuid1.IsNil())
	{
	  fprintf(stderr,"Error: argument must be a valid UUID\n");
	  exit(1);
	}
    }

  //
  // For options that take a pair of UUID's, split them up now, commas, slashes
  // and colons are all legitimate separators
  //

  if (opt_equals || opt_lessthan || opt_greaterthan)
    {
      if (uuid_args)
	{
	  char * token;
	  
	  token = strtok(uuid_args,",:/");
	  if (token) uuid1 = uuid(token);
	  token = strtok(NULL, ",:/");
	  if (token) uuid2 = uuid(token);

	  if (uuid1.IsNil() || uuid2.IsNil())
	    {
	      fprintf(stderr,"Error: one or both of the UUID arguments is invalid\n");
	      exit(1);
	    }
	}
      else
	{
	  fprintf(stderr,"Error: need UUID arguments \n");
	  exit(1);
	}
    }

    

  //
  // Dispatch the really easy functions
  //

  if (opt_idl_hdrgen)    mk_dce_idl_hdr();
  if (opt_com_initializer) mk_com_initializer();
  if (opt_c_initializer) mk_c_initializer();
  if (opt_testmsguid) test_ms_guid();
  if (opt_testdceuuid) test_dce_uuid();
  if (opt_lessthan) test_lt();
  if (opt_greaterthan) test_gt();
  if (opt_equals) test_eq();
  if (opt_test) test_dce_uuid();

  //
  // If no options, then generate N uuids
  //

  int i;
  for (i=0; i<num_uuids; i++) { fprintf(stdout,"%s\n",uuid().String()); }
  exit(0);

}


//
// mk_dce_idl_hdr():
//      Generate a header for a DCE RPC IDL interface file.
//
void
mk_dce_idl_hdr()
{
  uuid new_uuid;

  fprintf(stdout,
          "[ uuid(%s), version(1.0) ]\n"
          "interface INTERFACENAME\n"
          "{\n/* My interface definition goes here */\n}\n",
          new_uuid.String());

  exit(0);
}

//
// mk_c_initializer()
//      Generate an C-language initializer for a UUID.
//

void
mk_c_initializer()
{
  uuid new_uuid;
  uuid_t intl;
  int j; int v;

  intl = new_uuid.C_uuid();
  
  fprintf(stdout, 
          "uuid_t foo = { /* uuid (%s) */\n"
          "  0x%x, 0x%x, 0x%x, 0x%x, 0x%x,",
          new_uuid.String(),
          intl.time_low,
          intl.time_mid,
          intl.time_hi_and_version,
          (int)intl.clock_seq_hi_and_reserved,
          (int)intl.clock_seq_low );

 
  //
  // print the 6 byte node id 
  //
  
  fprintf(stdout, " { ");


  for (j=0; j<6; j++)
    {
      v = 0;
      v = intl.node[j];
      fprintf(stdout, "0x%x", v);

      if (j < 5) {
        fprintf(stdout, ", ");
      }
    }
  fprintf(stdout, " }");
  fprintf(stdout, "};\n");

  exit(0);
}

//
// mk_com_initializer()
//      Generate an C-language initializer for a Microsoft COM GUID.
//

void
mk_com_initializer()
{

  uuid new_uuid;
  uuid_t intl;
  int j; int v;

  intl = new_uuid.C_uuid();
  
  fprintf(stdout,
          "INTERFACENAME = { /* %s */\n"
          "  0x%x,\n"
          "  0x%x,\n"
          "  0x%x,\n"
          "  {0x%x, 0x%x, ",
          new_uuid.String(),
          intl.time_low,
          intl.time_mid,
          intl.time_hi_and_version,
          (int)intl.clock_seq_hi_and_reserved,
          (int)intl.clock_seq_low );


 
  //
  // print the 6 byte node id 
  //

  for (j=0; j<6; j++)
    {
      v = 0;
      v = intl.node[j];
      fprintf(stdout, "0x%x", v);
      if (j < 5) {
        fprintf(stdout, ", ");
      }
    }
  fprintf(stdout, "}");
  fprintf(stdout, "};\n");

  exit(0);
}

//
// Is a Microsoft GUID?
//

void
test_ms_guid()
{
  fprintf(stderr,"Error: MS GUID test presently not implemented.\n");
  exit(1);
}

//
// Is a DCE UUID?
//

void
test_dce_uuid()
{
  if (uuid1.IsNil())
    {
      fprintf(stderr,"is not a valid uuid.\n");
      exit(1);
    }
  else
    {
      exit(0);
    }
}

//
// Less Than comparator
//

void
test_lt()
{
  if (uuid1 < uuid2)
    {
      fprintf(stderr,"is less than.\n");
      exit(0);
    }
  else
    {
      exit(1);
    }
}

//
// Greater than comparator
//

void
test_gt()
{

  if (uuid1 > uuid2)
    {
      fprintf(stderr,"is greater than.\n");
      exit(0);
    }
  else
    {
      exit(1);
    }

}

//
// Equality comparator
//

void
test_eq()
{

  if (uuid1 == uuid2)
    {
      fprintf(stderr,"is equal to.\n");
      exit(0);
    }
  else
    {
      exit(1);
    }

}

