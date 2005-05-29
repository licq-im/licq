<?
  include "header.php";

  if (file_exists("/home/groups/l/li/licq/htdocs/zorbstats/zorblogs.php"))
    include("/home/groups/l/li/licq/htdocs/zorbstats/zorblogs.php");
  if (file_exists("/home/groups/l/li/licq/htdocs/zorbstats/zorblogspages.php"))
  {
    include ("/home/groups/l/li/licq/htdocs/zorbstats/zorblogspages.php");
    $pagename = substr($PHP_SELF, 1);
    $dispname = substr($PHP_SELF, 1, -4);
    countpage($pagename, $dispname);
  }
?>
      
      <div id="content" > 

				<div id="main"><img src="images/main_title_skins.gif" alt="Main" /></div>
        <div id="maincontent">
          <div id="subcontent">

          <p>Skins for Licq work with the Qt-GUI plugin and are easy to make (see the <a href="SKINS.HOWTO">SKINS.HOWTO</a>) and easy to install.</p>
          <p>To use one, simply unpack it into the qt-gui directory (/usr/local/share/licq/qt-gui or /usr/share/licq/qt-gui) or your licq directory (~/.licq) and then select it from the Skin Browser, or use the -s option to the qt-gui.</p> 
          <p>You can also test a skin by unpacking it anywhere and passing the gui the full pathname to the directory.</p>

          <h3><a href="skins.php">Skins</a> :: <a href="iconsets.php">Icons</a> :: Sounds :: Click Images to Zoom</h3> 
            
            <?

            $filetypes = array('.gz');

            $filelist = writetable('./skins/skin_files', $filetypes);

            function writetable($directory, $filetypes) 
            {
              //Table start
  
              $handle = opendir($directory);

              $nameList = file('skins/names.txt');
              foreach ($nameList as $key => $value)
                {
                  $nameList[$key] = trim($value);
                }

              $count =0;
              //Get filenames
              while (($file = readdir($handle))!==false)
              {

                if (strcmp(".",$file)!=0 && strcmp("..",$file)!=0)
                {
                  $filearray[$count] = $file;
                  $count++;
                }
              }   
              closedir($handle); 
              $count = 0;

              //Sort 'em by name
              sort($filearray);
              
              //Print list 
              $counter = 0;
              for ($i = 0; $i < count($filearray); $i++)
              { 
                $extension = strrchr($filearray[$i], ".");

                if (in_array($extension, $filetypes))
                {
                      
                    $pic = split("\.", $filearray[$i]);
                    foreach ($nameList as $name)
                    {
                      $temp = split(":", $name);
                      if ($temp[0] == $pic[1])
                      {
                        $author = $temp[1];
                      }
                    }
                    $list .= '<div style="text-align:center; float:left; width: 90px; height: 220px;"><a href="zoom.php?skin='. $pic[1] .'"><img src="skins/thumbs/' . $pic[1] . '.jpg" style="border: 0;" alt="'. $pic[1] .'"/></a><p style="font-size: 10px;"><a href="skins/skin_files/' . $filearray[$i] . '">' . $pic[1] . '</a></p><p style="font-size: 10px;">By: ' . $author . '</p></div>';
                    $counter++;
                  }
                 }
  
              return $list;
            }
          ?>
        
            <table style="text-align: center;" cellpadding="0" cellspacing="0">
              <tr>
                <td><? echo $filelist ?></td>
              </tr>
            </table>
      
          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
