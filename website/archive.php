<?

function news_last_id()
{
  $aux = file("news/last_id");
  return $aux[0];
}

function news_check_item($id)
{
  return is_readable("news/$id");
}

function news_read_item($id)
{
    return file("news/$id");
}

/* check if there is a previous item and return id else -1 */
function news_check_prev_item($cur_id)
{
  while ( $cur_id > 0 )
  {
        $cur_id--;
        if ( !news_check_item( $cur_id ) )
            continue;
        return $cur_id;
    }
    return -1;
}

/* check if there is a next item and return id else -1 */
function news_check_next_item( $cur_id )
{
    $last_id = news_last_id();
    while ( $cur_id < $last_id )
    {
        $cur_id++;
        if ( !news_check_item( $cur_id ) )
            continue;
        return $cur_id;
    }
    return -1;
}

function news_show_item( $data )
{
    $title = $data[1];
    $aux = split( "-", str_replace( ":", "-", str_replace( " ", "-", $data[0] ) ) );
    $date = date( "M jS G:i Y", mktime($aux[3],$aux[4],0,$aux[1],$aux[2],$aux[0] ) );
    ?>
    <div class="news">
      <h2><B><?echo $title;?></B> - <I><?echo $date;?></I></h2>
      <p>
      <? for ( $i =2; $i < count ( $data ); $i++ ) echo $data[$i]; ?>
      </p>
      <div class="footer">
        <div class="footer_right"></div>
      </div>
    </div>
    <?
}

function news_show_latest()
{
    $max_news_items = 10;
    $cur_id = news_last_id();
    settype( $cur_id, "integer" );
    /* read the latest valid newsitems from archive */
    $count = 0;
    while ( $count < $max_news_items && $cur_id >= 0 )
    {
        if ( news_check_item( $cur_id ) )
        {
          $aux = news_read_item( $cur_id );
          news_show_item( $aux );
          $count++;
        }
        $cur_id--;
    }
    echo "<center>[ <A href=\"news.php?action=show_news&news_action=show_archive\">News Archive</A> ]</center>";
}

function news_show_archive()
{
    $entries = dir_get_entries( "news" );
    echo "<P align=\"center\"><B>",count($entries)-2," news items found.</B></P>";
    echo "<DIV align=\"center\">";
    echo "<TABLE width=50% border=0 cellpadding=0 cellspacing=0><TR><TD>";
    $cur_id = news_last_id();
    settype( $cur_id, "integer" );
    while ( $cur_id >= 0 )
    {
        if ( news_check_item( $cur_id ) )
        {
            $aux = news_read_item( $cur_id );
            echo "<TABLE width=100% border=0 cellpadding=0 cellspacing=0>";
            echo "<TR><TD align=\"left\"><B><A href=\"news.php?action=show_news&news_action=show_item&item_id=$cur_id\">$aux[1]</A></B></TD><TD align=\"right\"><FONT size=-1>( $aux[0] )</FONT></TD></TR>";
            //echo "<TR><TD colspan=2 bgcolor=\"#000000\"><IMG alt=\"\" src=\"images/spacer1.gif\"></TD></TR>";
            echo "</TABLE>";
            echo "";
        }
        $cur_id--;
    }
    echo "</TD></TR></TABLE>";
    echo "</DIV>";
}

function news_show_single_item()
{
    global $item_id;
    $data = news_read_item( $item_id );
    news_show_item( $data );
    $prev_id = news_check_prev_item( $item_id );
    $next_id = news_check_next_item( $item_id );
    echo "<center>";
    if ( $prev_id != -1 )
        echo "[ <A href=\"news.php?news_action=show_item&item_id=$prev_id\">Prev</A> ]";
    echo "[ <A href=\"news.php?news_action=show_archive\">News Archive</A> ]";
    if ( $next_id != -1 )
        echo "[ <A href=\"news.php?news_action=show_item&item_id=$next_id\">Next</A> ]";
    echo "</center>";
}

/*
 * Get all directory entries except . and ..
 */
function dir_get_entries( $path ) {
    $i = 0;
    $hdir = opendir( $path );
    while( $entry = readdir( $hdir ) ) {
        if ( $entry == "." ) continue;
        if ( $entry == ".." ) continue;
        $data[$i++] = $entry;
    }
    closedir( $hdir );
    if ( !empty( $data ) ) sort( $data );
    return $data;
}
?>
