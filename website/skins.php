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
      
