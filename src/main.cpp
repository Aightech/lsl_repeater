#include <vector>
#include <fstream>
#include <iostream>
#include <stdlib.h> 
#include <unistd.h>
#ifdef WIN32
#include <time.h>
#endif
#include <lsl_cpp.h>
#include "tools.h"

/**
 * @brief scanstream Search for the lsl streams and add them to the lslinfo vector..
 * @param to Lslinfo vector.
 * @param verbose To activate the display of the parameters of each streams.
 */
void scanStream(std::vector<lsl::stream_info>& to, bool verbose=true)
{
  std::vector<lsl::stream_info> strm_info = lsl::resolve_streams();
  if(strm_info.size()>0)
    {
      std::string channel_format_str[9] { "none",
            "cf_float32",
            "cf_double64",
            "cf_string",
            "cf_int32",
            "cf_int16",
            "cf_int8",
            "cf_int64",
            "cf_undefined"     // Can not be transmitted.
           };
      
      std::cout << "[INFO] Available Streams:" << std::endl;
      for (int i = 0; i<strm_info.size(); i++)
	{
	  std::cout <<"-  " <<  strm_info[i].name()  << " [ "<< strm_info[i].uid() << " ]" << std::endl;
	  if(verbose)
	    {
	      
	      std::cout <<"   > Type:         " <<  strm_info[i].type() << std::endl;
	      std::cout <<"   > Nb. Channels: " <<  strm_info[i].channel_count() << std::endl;
	      std::cout <<"   > Format:       " <<  channel_format_str[strm_info[i].channel_format()] << std::endl;
	      std::cout <<"   > Host name:    " <<  strm_info[i].hostname() << std::endl;	  
	      std::cout <<"   > Rate:         " <<  strm_info[i].nominal_srate() << std::endl;
	      //channel_format_str[m_results[0].channel_format()];
	    }

	  //search for the stream in the already scanned ones
	  int new_strm = true;
	  for(int j =0; j < to.size(); j++)
	    {
	      if(strm_info[i].uid().compare( to[j].uid() ) == 0 )
		new_strm = false;
	    }
	  if(new_strm)
	    to.push_back(strm_info[i]);
	}
    }


}


int main(int argc, char ** argv)
{
  std::vector<std::string> opt_flag(
  				    {"-n_in",
  					"-n_out",
  					"-host"});
  std::vector<std::string> opt_label(
  				     {"Lsl repeated stream's name",
  					 "Lsl repeating stream's name",
  					 "Nane of the stream host"});
  std::vector<std::string> opt_value(
  				     {"Right_Hand_Command",
  					 "//",
  					 "Ringo"});
  get_arg(argc, argv, opt_flag, opt_label, opt_value);
  
  std::string stream_in_name = opt_value[0];
  std::string stream_out_name = (opt_value[1].compare("//")==0)?opt_value[0]:opt_value[1] ;
  std::string host_name = opt_value[2];

  //scan the available streams.
  std::vector<lsl::stream_info> strm_info;
  scanStream(strm_info, false);
  bool found_stream=false;
  int index_stream =-1;
  for(int i =0; i < strm_info.size(); i++)
    {
      if(strm_info[i].name().compare( stream_in_name ) == 0 )
	    {//launch a thread
	      found_stream=true;
	      index_stream=i;
	      std::cout << "[INFO] Start to record: " <<  strm_info[i].name() << std::endl;
	    }
    }

  
  
  if(found_stream)
    {
      std::string type =  strm_info[index_stream].type();
      int nb_ch = strm_info[index_stream].channel_count();
      lsl::channel_format_t format = strm_info[index_stream].channel_format(); 
      int rate = strm_info[index_stream].nominal_srate();
      lsl::stream_info info_out(stream_out_name, type, nb_ch, rate, format);
      try
	{
	  lsl::stream_inlet inlet(strm_info[index_stream]);
	  lsl::stream_outlet outlet(info_out);

	  std::vector<std::vector<float>> chunk;
	  std::vector<double> timestamps;
	  while(1)
	    if (inlet.pull_chunk(chunk, timestamps))	
	      outlet.push_chunk(chunk, timestamps);

	}
      catch (std::exception& e)
	{
	  std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
	}
    }

  
    
  return 0;
}
