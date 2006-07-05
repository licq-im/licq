<?
  include "header.php";
  include "archive.php";

$news_action = $_REQUEST['news_action']
    if ( empty( $news_action ) ) $news_action = "show_archive";
?>
      
      <div id="content" > 

				<div id="main"><img src="images/main_title_news.gif" alt="News" /></div>
        <div id="maincontent">
          <div id="subcontent">
            
            <p>Check out the <a href="CHANGELOG">CHANGELOG</a> for a more detailed list of what has changed.</p>


<?
  echo "<P>";
  switch ($news_action)
  {
    case "show_latest": 
        news_show_latest(); 
        break;
    case "show_archive": news_show_archive(); break;
    case "show_item": news_show_single_item(); break;
  }
  echo "</P>";
?>

          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
