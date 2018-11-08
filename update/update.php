<?php
	$ServerName="www.simpai.net";
	$action=strtolower($_GET[action]);
	
	if(eregi($action,"auto")) 
	{
		echo "http://".$ServerName."/update/update_info.xml";
	}
	
	if(eregi($action,"manual")) 
	{
		echo "http://".$ServerName."/update/update_info.xml";
	}
?>
