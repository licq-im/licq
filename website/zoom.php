<?
  include "header.php";

  $skin = $_GET['skin'];

  $display = '';
  $display .= '<div style="text-align: center;"><img src="skins/fullsize/'. $skin .'.jpg" border="0" />';
  
?>

      <div id="content" > 

				<div id="main"><img src="images/main_title_skins.gif" alt="Skins" /></div>
        <div id="maincontent">
          <div id="subcontent">
          <p>Skin Name: <? echo $skin ?></a> :: <a href="skins/skin_files/skin.<? echo $skin ?>.tar.gz">Download</a></p> 
          <p><a href="skins.php">Back to Skins</a></p>

          <? echo $display ?>

          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p></div>

			</div> 

<? include "footer.php" ?>
