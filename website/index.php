<?
  include "header.php";
  include "archive.php";
    include("/var/www/htdocs/zorbstats/zorblogs.php");

    include ("/var/www/htdocs/zorbstats/zorblogspages.php");
    $pagename = "licq".substr($PHP_SELF,1);
    $dispname = "licq".substr($PHP_SELF,1,-4);
    countpage($pagename,$dispname);

?>

      <div id="content" > 

				<div id="main"><img src="images/main_title_main.gif" alt="Main" /></div>
        <div id="maincontent">
          <div id="subcontent">
            <h2>Welcome to the Licq homepage.</h2>
            <p>Latest Update: <b>October 21, 2005</b><br />Stable Version: <b>1.3.2</b><br />Devel Version: <b>1.3.2</b></p>
            
<a href="http://sourceforge.net/donate/index.php?group_id=254"><img src="http://images.sourceforge.net/images/project-support.jpg" width="88" height="32" border="0" alt="Support This Project" /> </a>
<? 
   require_once('rss_fetch.inc');
   $rss = fetch_rss("http://sourceforge.net/export/rss2_projdonors.php?group_id=254");
   echo "Latest donors: ";
   if ($rss->items)
   {
     foreach ($rss->items as $item)
     {
       $title = $item['title'];
       $desc = $item['description'];
       echo $title;
       if ($desc)
         echo "(<i>" . $desc . "</i>)";
       echo ", ";
     }
    }
    else
      echo $rss->channel['description'];

    news_show_latest(); 
?>

          </div>
        </div>


        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
