<!DOCTYPE TS><TS>
<context>
    <name></name>
    <message>
        <source>none</source>
        <translation type="obsolete">なし</translation>
    </message>
</context>
<context>
    <name>@default</name>
    <message>
        <source>KDE default</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AddUserDlg</name>
    <message>
        <source>Licq - Add User</source>
        <translation>Licq - ユーザ追加</translation>
    </message>
    <message>
        <source>New User UIN:</source>
        <translation type="obsolete">新ユーザUIN</translation>
    </message>
    <message>
        <source>&amp;Alert User</source>
        <translation type="obsolete">ユーザ通知</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New User ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
</context>
<context>
    <name>AuthUserDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Refuse authorization to %1</source>
        <translation>%1の承認を拒否する</translation>
    </message>
    <message>
        <source>Authorize which user (UIN):</source>
        <translation type="obsolete">どのユーザを承認(UIN):</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias (such as &quot;&gt;&quot; or &quot;&amp;&quot;</source>
        <translation type="obsolete">&lt;h2&gt;自動応答メッセージ&lt;br&gt;作成のヒント&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;%による表記(メインのヒントページに詳細があります)を含めることができます。&lt;/li&gt;&lt;li&gt;パイプ(|)から始まる行はコマンドと見なされます。 その行に指定されたコマンドは外部コマンドに置き換えられます。 コマンドは/bin/shによって解釈されるのでシェルコマンドあるいはメタ文字を 使用することができます。セキュリティ上の理由から%による表現は エリアス(&quot;&gt;&quot;や&quot;&amp;&quot;)に含まれるメタ文字をシェルが解釈しないよう 自動的にシングルクオートで囲まれた形に変換されます。</translation>
    </message>
    <message>
        <source>Grant authorization to %1</source>
        <translation>%1を承認する</translation>
    </message>
    <message>
        <source>Licq - Grant Authorisation</source>
        <translation>承認を認める</translation>
    </message>
    <message>
        <source>Licq - Refuse Authorisation</source>
        <translation>Licq - 承認を拒否</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Response</source>
        <translation>応答</translation>
    </message>
    <message>
        <source>Authorize which user (Id):</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>I&apos;m currently %1, %a.
You can leave me a message.
(%m messages pending from you).</source>
        <translation>只今席を外しております、%a
メッセージを残してください。
(%mのメッセージがあります)</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>ヒント</translation>
    </message>
    <message>
        <source>&amp;Edit Items</source>
        <translation>アイテム編集</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>&amp;Select</source>
        <translation>選択</translation>
    </message>
    <message>
        <source>Set %1 Response for %2</source>
        <translation>%2への応答を%1にセット</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Setting&lt;br&gt;your Auto-Response&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;You can include any of the % expansions (described in the main hints page).&lt;/li&gt;&lt;li&gt;Any line beginning with a pipe (|) will be treated as a command to be run.  The line will be replaced by the output of the command. The command is parsed by /bin/sh so any shell commands or meta-characters are allowed.  For security reasons, any % expansions are automatically passed to the command surrounded by single quotes to prevent shell parsing of any meta-characters included in an alias.&lt;br&gt;Examples of popular uses include:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;|date&lt;/tt&gt;: Will replace that line by the current date&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|fortune&lt;/tt&gt;: Show a fortune, as a tagline for example&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a&lt;/tt&gt;: Run a script, passing the uin and alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|myscript.sh %u %a &gt; /dev/null&lt;/tt&gt;: Run the same script but ignore the output (for tracking auto response checks or something)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;|if [ %u -lt 100000 ]; then echo &quot;You are special&quot;; fi&lt;/tt&gt;: Useless, but shows how you can use shell script.&lt;/li&gt;&lt;/ul&gt;Of course, multiple &quot;|&quot; can appear in the auto response, and commands and regular text can be mixed line by line.&lt;/li&gt;&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(Closing in %1)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CEditFileListDlg</name>
    <message>
        <source>Licq - Files to send</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation type="unfinished">消去</translation>
    </message>
</context>
<context>
    <name>CEmoticons</name>
    <message>
        <source>Default</source>
        <translation type="unfinished">デフォルト</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CFileDlg</name>
    <message>
        <source>KB</source>
        <translation>KB</translation>
    </message>
    <message>
        <source>MB</source>
        <translation>MB</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>Byte</source>
        <translation>バイト</translation>
    </message>
    <message>
        <source>ETA:</source>
        <translation>残り時間:</translation>
    </message>
    <message>
        <source>%1/%2</source>
        <translation>%1/%2</translation>
    </message>
    <message>
        <source>Bytes</source>
        <translation>バイト</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>ファイル:</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>もう一度</translation>
    </message>
    <message>
        <source>Time:</source>
        <translation>時間:</translation>
    </message>
    <message>
        <source>Remote side disconnected</source>
        <translation type="obsolete">相手が切断しました</translation>
    </message>
    <message>
        <source>Licq - File Transfer (%1)</source>
        <translation>Licq - ファイル転送(%1)</translation>
    </message>
    <message>
        <source>Remote side disconnected
</source>
        <translation type="obsolete">相手が切断しました
</translation>
    </message>
    <message>
        <source>Sent
%1
to %2 successfully
</source>
        <translation type="obsolete">%1を
%2に
送信成功</translation>
    </message>
    <message>
        <source>Connecting to remote...
</source>
        <translation type="obsolete">相手に接続中...</translation>
    </message>
    <message>
        <source>Received
%1
from %2 successfully
</source>
        <translation type="obsolete">%2を
%1より
受信に成功
</translation>
    </message>
    <message>
        <source>File already exists and appears incomplete.</source>
        <translation>ファイルは既に存在し不完全の模様</translation>
    </message>
    <message>
        <source>Batch:</source>
        <translation>バッチ:</translation>
    </message>
    <message>
        <source>File already exists and is at least as big as the incoming file.</source>
        <translation>ファイルは既に存在し転送されるファイルよりサイズが大</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Waiting for connection...
</source>
        <translation type="obsolete">接続待ち...
</translation>
    </message>
    <message>
        <source>Sending file...
</source>
        <translation type="obsolete">ファイルを送り中...
</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>再開</translation>
    </message>
    <message>
        <source>Done %1
</source>
        <translation type="obsolete">終了 %1
</translation>
    </message>
    <message>
        <source>File name:</source>
        <translation>ファイル名:</translation>
    </message>
    <message>
        <source>Receiving file...
</source>
        <translation type="obsolete">ファイルを受信中...
</translation>
    </message>
    <message>
        <source>File I/O error: %1
</source>
        <translation type="obsolete">ファイルI/Oエラー: %1
</translation>
    </message>
    <message>
        <source>Handshake Error
See Network Window for Details</source>
        <translation type="obsolete">ハンドシェークエラー
詳細はネットワークウィンドウを参照ください</translation>
    </message>
    <message>
        <source>File transfer cancelled
</source>
        <translation type="obsolete">ファイル転送キャンセル
</translation>
    </message>
    <message>
        <source>File I/O Error:
%1
See Network Window for Details</source>
        <translation type="obsolete">ファイルI/Oエラー:
%1
詳細はネットワークウィンドウを御覧ください</translation>
    </message>
    <message>
        <source>Handshaking error
</source>
        <translation type="obsolete">ハンドシェイクエラー
</translation>
    </message>
    <message>
        <source>Current:</source>
        <translation>現在</translation>
    </message>
    <message>
        <source>Open error - unable to open file for writing.</source>
        <translation>オープンエラー - ファイルに書き込めません</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>上書き</translation>
    </message>
    <message>
        <source>&amp;Cancel Transfer</source>
        <translation>転送キャンセル</translation>
    </message>
    <message>
        <source>File transfer complete
</source>
        <translation type="obsolete">ファイル転送完了
</translation>
    </message>
    <message>
        <source>File transfer cancelled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Waiting for connection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Receiving file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sending file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Received %1 from %2 successfully.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sent %1 to %2 successfully.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File transfer complete.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>Remote side disconnected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File I/O error: %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File I/O Error:
%1

See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Handshaking error.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Handshake Error.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection error.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to reach remote host.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Bind error.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not enough resources.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to create a thread.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connecting to remote...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation type="unfinished">開く</translation>
    </message>
    <message>
        <source>O&amp;pen Dir</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CFontDialog</name>
    <message>
        <source>Select Font</source>
        <translation type="obsolete">フォント選択</translation>
    </message>
</context>
<context>
    <name>CForwardDlg</name>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>メッセージ</translation>
    </message>
    <message>
        <source>Forward %1 To User</source>
        <translation>ユーザに %1 を転送</translation>
    </message>
    <message>
        <source>Drag the user to forward to here:</source>
        <translation>転送先ユーザをここにドラッグして下さい</translation>
    </message>
    <message>
        <source>Unable to forward this message type (%d).</source>
        <translation>このメッセージタイプ(%d)を転送することは無理です</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Forwarded message:
</source>
        <translation>転送されたメッセージ:
</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>転送</translation>
    </message>
    <message>
        <source>Forwarded URL:
</source>
        <translation>転送された URL:
</translation>
    </message>
</context>
<context>
    <name>CInfoField</name>
    <message>
        <source>Unknown</source>
        <translation type="unfinished">未知</translation>
    </message>
</context>
<context>
    <name>CJoinChatDlg</name>
    <message>
        <source>Select chat to join:</source>
        <translation>参加するチャットを選択</translation>
    </message>
    <message>
        <source>Invite to Join Chat</source>
        <translation>チャットに誘う</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Select chat to invite:</source>
        <translation>誘うチャットを選択</translation>
    </message>
    <message>
        <source>Join Multiparty Chat</source>
        <translation>複数人チャットに参加</translation>
    </message>
    <message>
        <source>&amp;Invite</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation type="unfinished">参加</translation>
    </message>
</context>
<context>
    <name>CLicqMessageBox</name>
    <message>
        <source>Licq</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Next</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Next (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation type="unfinished">Licq情報</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation type="unfinished">Licq警告</translation>
    </message>
    <message>
        <source>Licq Critical</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CMMSendDlg</name>
    <message>
        <source>Sending mass message to %1...</source>
        <translation>%1にメッセージを送信中...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Sending mass URL to %1...</source>
        <translation>%1にURLを送信中...</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>失敗</translation>
    </message>
    <message>
        <source>Multiple Recipient Contact List</source>
        <translation>複数人受信コンタクトリスト</translation>
    </message>
    <message>
        <source>Multiple Recipient URL</source>
        <translation>複数人受信URL</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Sending mass list to %1...</source>
        <translation>%1にリストを送信中...</translation>
    </message>
    <message>
        <source>Multiple Recipient Message</source>
        <translation>複数人受信メッセージ</translation>
    </message>
</context>
<context>
    <name>CMMUserView</name>
    <message>
        <source>Crop</source>
        <translation>落す</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>クリア</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>除去</translation>
    </message>
    <message>
        <source>Add Group</source>
        <translation>グループを追加</translation>
    </message>
    <message>
        <source>Add All</source>
        <translation>全て加える</translation>
    </message>
    <message>
        <source>Drag&apos;n&apos;Drop didn&apos;t work</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CMainWindow</name>
    <message>
        <source>s</source>
        <translation></translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation>No</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>はい</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>送信</translation>
    </message>
    <message>
        <source>Online to User</source>
        <translation>ユーザにオンライン</translation>
    </message>
    <message>
        <source>&amp;Away</source>
        <translation>離席</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>ヘルプ</translation>
    </message>
    <message>
        <source>&amp;Info</source>
        <translation>情報</translation>
    </message>
    <message>
        <source>U&amp;tilities</source>
        <translation>ユーティリティ</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>&amp;Options...</source>
        <translation>オプション</translation>
    </message>
    <message>
        <source>R&amp;andom Chat</source>
        <translation>ランダムチャット</translation>
    </message>
    <message>
        <source>Error sending authorization.</source>
        <translation type="obsolete">承認送信時にエラー</translation>
    </message>
    <message>
        <source>&amp;Random Chat Group</source>
        <translation>ランダムチャットのグループ</translation>
    </message>
    <message>
        <source>Up since %1
</source>
        <translation>%1より上がっています
</translation>
    </message>
    <message>
        <source>Check %1 Response</source>
        <translation>%1応答をチェック</translation>
    </message>
    <message>
        <source>You are currently registered as
UIN: %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation type="obsolete">あなたは現在UIN: %1
で登録しています。
基本ディレクトリ: %2
別ディレクトリを選択するにはlicqに-bオプションを付けて
再起動し、新ユーザとして登録してください。</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1.</source>
        <translation>アイコンファイルを開けません
%1</translation>
    </message>
    <message>
        <source>Send &amp;Authorization</source>
        <translation>承認を送信</translation>
    </message>
    <message>
        <source>&amp;Not Available</source>
        <translation>忙しい</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>Licqについて</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>グループ</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>ヒント</translation>
    </message>
    <message>
        <source>&amp;Reset</source>
        <translation>リセット</translation>
    </message>
    <message>
        <source>Custom Auto Response...</source>
        <translation>自動応答編集</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation type="obsolete">オンライン通知</translation>
    </message>
    <message>
        <source>Set &amp;Auto Response...</source>
        <translation>自動応答をセット</translation>
    </message>
    <message>
        <source>Remove From List</source>
        <translation>リストから除去</translation>
    </message>
    <message>
        <source>System Message</source>
        <translation>システムメッセージ</translation>
    </message>
    <message>
        <source>%1 msg%2</source>
        <translation>%1 メッセージ%2</translation>
    </message>
    <message>
        <source>&amp;Popup All Messages</source>
        <translation>全メッセージをポップアップ</translation>
    </message>
    <message>
        <source>Accept in Not Available</source>
        <translation>忙しいとき受け付ける</translation>
    </message>
    <message>
        <source>View &amp;History</source>
        <translation>履歴を見る</translation>
    </message>
    <message>
        <source>Other Users</source>
        <translation>他ユーザ</translation>
    </message>
    <message>
        <source>Edit User Group</source>
        <translation>ユーザグループを編集</translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation type="obsolete">登録に成功しました。あなたのICQ番号(UIN)
は %1 です。
次にあなたの個人情報を設定してください。</translation>
    </message>
    <message>
        <source>Unknown Packets</source>
        <translation>未知パケット</translation>
    </message>
    <message>
        <source>Send &amp;Url</source>
        <translation type="obsolete">URL送信</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>&lt;hr&gt;&lt;p&gt; For more information, see the Licq webpage (&lt;tt&gt;http://www.licq.org&lt;/tt&gt;).&lt;/p&gt;</source>
        <translation>さらに詳しい情報はLicqのホームページ(&lt;tt&gt;http://www.licq.org&lt;/tt&gt;)を御覧ください。</translation>
    </message>
    <message>
        <source>&amp;Mini Mode</source>
        <translation>ミニモード</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>Close &amp;Secure Channel</source>
        <translation>安全路を閉じる</translation>
    </message>
    <message>
        <source>No msgs</source>
        <translation>メッセージなし</translation>
    </message>
    <message>
        <source>Licq version %1%8.
Qt GUI plugin version %2.
Compiled on: %7
%6
Author: Graham Roff
Contributions by Dirk A. Mueller
http://www.licq.org

%3 (%4)
%5 contacts.</source>
        <translation type="obsolete">Licq バージョン %1%8.
Qt GUI プラグインバージョン %2.
%7にコンパイル
%6
作者: Graham Roff
Dirk A. Muellerさん Thanks.
http://www.licq.org

%3 (%4)
コンタクト数 %5</translation>
    </message>
    <message>
        <source>Up since %1

</source>
        <translation>%1より上がっています

</translation>
    </message>
    <message>
        <source>Daemon Statistics

</source>
        <translation>デーモン統計

</translation>
    </message>
    <message>
        <source>Daemon Statistics
(Today/Total)
</source>
        <translation>デーモン統計
(Today/Total)
</translation>
    </message>
    <message>
        <source>SysMsg</source>
        <translation>システムメッセージ</translation>
    </message>
    <message>
        <source>System</source>
        <translation>システム</translation>
    </message>
    <message>
        <source>Last reset %1

</source>
        <translation>最後のリセット %1

</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from the &apos;%3&apos; group?</source>
        <translation>本当に
%1 (%2)
をグループ「%3」から削除しますか?</translation>
    </message>
    <message>
        <source>Do you really want to add
%1 (%2)
to your ignore list?</source>
        <translation>本当に
%1 (%2)
を無視リストに追加しますか?</translation>
    </message>
    <message>
        <source>Packets</source>
        <translation>パケット</translation>
    </message>
    <message>
        <source>&amp;Save All Users</source>
        <translation>全ユーザをセーブ</translation>
    </message>
    <message>
        <source>%1: %2 / %3
</source>
        <translation>%1: %2 / %3
</translation>
    </message>
    <message>
        <source>Do Not Disturb to User</source>
        <translation>ユーザに対して邪魔しないで♪</translation>
    </message>
    <message>
        <source>Update All Users</source>
        <translation>全ユーザを更新</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:
</source>
        <translation type="obsolete">&lt;h2&gt;Licq Qt-GUIプラグイン&lt;br&gt;使用のためのヒント&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;ステータスラベルを右クリックして自分のステータスを変えることができます。&lt;/li&gt;&lt;li&gt;ステータスラベルをダブルクリックすることで自分の自動応答メッセージを変えられます。&lt;/li&gt;&lt;li&gt;メッセージラベルをダブルクリックするとシステムメッセージを閲覧できます。&lt;/li&gt;&lt;li&gt;メッセージラベルを右クリックするとグループを変えることができます。&lt;/li&gt;&lt;li&gt;コンタクトリストからは下記のショートカットを使用できます。&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;ミニモードへ&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;オフラインユーザを隠す&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;終了&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;画面を隠す&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;次のメッセージを見る&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;メッセージを見る&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;メッセージを送信&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;URLを送信&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;チャットリクエストを送信Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;ファイルを送信Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;自動応答をチェック&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;全メッセージをポップアップ&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;ユーザウィンドウを再描画&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;現在のグループからユーザを削除&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;コンタクトリストからユーザを削除&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;コンタクトリストからユーザを削除する時は閉じるをクリックするまで   コントロールキーを押したままにしてください。&lt;/li&gt;&lt;li&gt;「OK」あるいは「受信」を選ぶときはコントロールキー押しながらエンターですみます。   その一例としては、ユーザ機能ウィンドウの送信タブがあげられます。&lt;/li&gt;&lt;li&gt;下記は&lt;b&gt;OnEvent&lt;/b&gt;パラメータ、&lt;b&gt;自動応答&lt;/b&gt;、&lt;b&gt;ユーティリティ&lt;/b&gt;として   使用できる%表記の一覧です。
</translation>
    </message>
    <message>
        <source>&amp;Redraw User Window</source>
        <translation>ユーザウィンドウを再描画</translation>
    </message>
    <message>
        <source>(with KDE support)
</source>
        <translation>(KDEサポートあり)
</translation>
    </message>
    <message>
        <source>Occupied to User</source>
        <translation>ユーザに対して用事中</translation>
    </message>
    <message>
        <source>Logon failed.
See network window for details.</source>
        <translation>ログインに失敗しました。
詳細はネットワークウィンドウを御覧ください。</translation>
    </message>
    <message>
        <source>Accept in Away</source>
        <translation>離席中に受け付ける</translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation type="obsolete">不可視リスト</translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation type="obsolete">可視リスト</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation type="obsolete">登録に失敗しました。詳細はネットワークウィンドウを御覧ください</translation>
    </message>
    <message>
        <source>Send &amp;Chat Request</source>
        <translation>チャットリクエスト送信</translation>
    </message>
    <message>
        <source>Reg&amp;ister User</source>
        <translation type="obsolete">登録済ユーザ</translation>
    </message>
    <message>
        <source>Show Offline &amp;Users</source>
        <translation>オフラインユーザも呈示</translation>
    </message>
    <message>
        <source>%1 message%2</source>
        <translation>%1 メッセージ%2</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation type="obsolete">無視リスト</translation>
    </message>
    <message>
        <source>%1: %2
</source>
        <translation>%1: %2
</translation>
    </message>
    <message>
        <source>&amp;More Info</source>
        <translation type="obsolete">その他の情報</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <source>&amp;Work Info</source>
        <translation type="obsolete">仕事の情報</translation>
    </message>
    <message>
        <source>Debug Level</source>
        <translation>デバッグレベル</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation type="obsolete">新ユーザ</translation>
    </message>
    <message>
        <source>&amp;Network Window</source>
        <translation>ネットワークウィンドウ</translation>
    </message>
    <message>
        <source>&amp;View Event</source>
        <translation>イベントを見る</translation>
    </message>
    <message>
        <source>&amp;Plugin Manager...</source>
        <translation>プラグインマネージャ</translation>
    </message>
    <message>
        <source>O&amp;ccupied</source>
        <translation>用事中</translation>
    </message>
    <message>
        <source>Set All</source>
        <translation>全てセット</translation>
    </message>
    <message>
        <source>&amp;View System Messages</source>
        <translation>システムメッセージを見る</translation>
    </message>
    <message>
        <source>&amp;Thread Group View</source>
        <translation>スレッドグループ表示</translation>
    </message>
    <message>
        <source>Send &amp;Message</source>
        <translation>メッセージ送信</translation>
    </message>
    <message>
        <source>Update Current Group</source>
        <translation>現グループを更新</translation>
    </message>
    <message>
        <source>Licq (%1)</source>
        <translation>Licq (%1)</translation>
    </message>
    <message>
        <source>Right click - Status menu
Double click - Set auto response</source>
        <translation>右クリック - ステータスメニュー
ダブルクリック - 自動応答を設定</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>ユーザ追加</translation>
    </message>
    <message>
        <source>Accept in Do Not Disturb</source>
        <translation>じゃましないで♪の時受け付ける</translation>
    </message>
    <message>
        <source>&amp;Security Options</source>
        <translation type="obsolete">セキュリティオプション</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>統計</translation>
    </message>
    <message>
        <source>&amp;Online</source>
        <translation>オンライン</translation>
    </message>
    <message>
        <source>&amp;Away Modes</source>
        <translation type="obsolete">離席モード</translation>
    </message>
    <message>
        <source>Clear All</source>
        <translation>全てクリア</translation>
    </message>
    <message>
        <source>&amp;Status</source>
        <translation>ステータス</translation>
    </message>
    <message>
        <source>&amp;System</source>
        <translation>システム</translation>
    </message>
    <message>
        <source>Accept in Occupied</source>
        <translation>用事中でも受け付ける</translation>
    </message>
    <message>
        <source>Send Contact &amp;List</source>
        <translation>コンタクトリスト送信</translation>
    </message>
    <message>
        <source>Status Info</source>
        <translation>ステータス情報</translation>
    </message>
    <message>
        <source>&amp;Do Not Disturb</source>
        <translation>じゃましないで♪</translation>
    </message>
    <message>
        <source>Change &amp;Password</source>
        <translation type="obsolete">パスワードを変える</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
%1 (%2)
from your contact list?</source>
        <translation>本当に
%1 (%2)
をコンタクトリストからはずしますか?</translation>
    </message>
    <message>
        <source>O&amp;ffline</source>
        <translation>オフライン</translation>
    </message>
    <message>
        <source>A&amp;uthorize User</source>
        <translation>ユーザを承認</translation>
    </message>
    <message>
        <source>Free for C&amp;hat</source>
        <translation>チャットできます</translation>
    </message>
    <message>
        <source>Right click - User groups
Double click - Show next message</source>
        <translation>右クリック - ユーザグループ
ダブルクリック - 次メッセージを表示</translation>
    </message>
    <message>
        <source>No messages</source>
        <translation>メッセージなし</translation>
    </message>
    <message>
        <source>&amp;Invisible</source>
        <translation>不可視</translation>
    </message>
    <message>
        <source>&amp;General Info</source>
        <translation type="obsolete">一般の情報</translation>
    </message>
    <message>
        <source>Send &amp;File Transfer</source>
        <translation>ファイル転送送信</translation>
    </message>
    <message>
        <source>User Functions</source>
        <translation>ユーザ機能</translation>
    </message>
    <message>
        <source>Check Auto Response</source>
        <translation>自動応答をチェック</translation>
    </message>
    <message>
        <source>Authorization granted.</source>
        <translation type="obsolete">承認は受け付けられました。</translation>
    </message>
    <message>
        <source>System Functions</source>
        <translation>システム機能</translation>
    </message>
    <message>
        <source>Edit &amp;Groups</source>
        <translation>グループを編集</translation>
    </message>
    <message>
        <source>All Users</source>
        <translation type="obsolete">全ユーザ</translation>
    </message>
    <message>
        <source>S&amp;kin Browser...</source>
        <translation>スキンブラウザ</translation>
    </message>
    <message>
        <source>Request &amp;Secure Channel</source>
        <translation>安全路を要求</translation>
    </message>
    <message>
        <source>S&amp;earch for User</source>
        <translation>ユーザを探す</translation>
    </message>
    <message>
        <source>Next &amp;Server</source>
        <translation type="obsolete">次のサーバ</translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="obsolete">&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;ユーザalias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;メールアドレス&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;名&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;電話番号&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;IPアドレス&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;姓&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;たまっているメッセージ数&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;フルネーム&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;最後のオンライン&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;ユーザポートuser port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;フルステイタス&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;簡略ステイタス&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;UIN&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;ホームページ&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation type="obsolete">過去ログ</translation>
    </message>
    <message>
        <source>Not Available to User</source>
        <translation>ユーザに対して忙しい</translation>
    </message>
    <message>
        <source>Toggle &amp;Floaty</source>
        <translation>浮遊モード</translation>
    </message>
    <message>
        <source>Sa&amp;ve Settings</source>
        <translation>設定をセーブ</translation>
    </message>
    <message>
        <source>Away to User</source>
        <translation>ユーザに対して離席</translation>
    </message>
    <message>
        <source> </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send &amp;URL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send Authorization Re&amp;quest</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send &amp;SMS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation type="obsolete">最後</translation>
    </message>
    <message>
        <source>&amp;Security/Password Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Re&amp;quest Authorization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Accept Files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Accept Chats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Request Secure</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use Real Ip (LAN)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Misc Modes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error! No owner set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;%1&lt;/b&gt; is online</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Server Group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(Error! No owner set)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq version %1%8.
Qt GUI plugin version %2.
Compiled on: %7
%6
Maintainer: Jon Keating
Contributions by Dirk A. Mueller
Original Author: Graham Roff

http://www.licq.org
#licq on irc.freenode.net

%3 (%4)
%5 contacts.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;GPG Key Manager...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Don&apos;t Show</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Available</source>
        <translation type="unfinished">利用可能</translation>
    </message>
    <message>
        <source>Busy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Phone &quot;Follow Me&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Owner Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update Info Plugin List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update Status Plugin List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update Phone &quot;Follow Me&quot; Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update ICQphone Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update File Server Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set GPG key</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Do you really want to reset your stats?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;ul&gt;&lt;li&gt;&lt;tt&gt;%a - &lt;/tt&gt;user alias&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%e - &lt;/tt&gt;email&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%f - &lt;/tt&gt;first name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%h - &lt;/tt&gt;phone number&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%i - &lt;/tt&gt;user ip&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%l - &lt;/tt&gt;last name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%L - &lt;/tt&gt;local time&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%m - &lt;/tt&gt;# pending messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%M - &lt;/tt&gt;# pending messages (if any)&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%n - &lt;/tt&gt;full name&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%o - &lt;/tt&gt;last seen online&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%O - &lt;/tt&gt;online since&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%p - &lt;/tt&gt;user port&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%s - &lt;/tt&gt;full status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%S - &lt;/tt&gt;abbreviated status&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%u - &lt;/tt&gt;uin&lt;/li&gt;&lt;li&gt;&lt;tt&gt;%w - &lt;/tt&gt;webpage&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;h2&gt;Hints for Using&lt;br&gt;the Licq Qt-GUI Plugin&lt;/h2&gt;&lt;br&gt;&lt;hr&gt;&lt;br&gt;&lt;ul&gt;&lt;li&gt;Change your status by right clicking on the status label.&lt;/li&gt;&lt;li&gt;Change your auto response by double-clicking on the status label.&lt;/li&gt;&lt;li&gt;View system messages by double clicking on the message label.&lt;/li&gt;&lt;li&gt;Change groups by right clicking on the message label.&lt;/li&gt;&lt;li&gt;Use the following shortcuts from the contact list:&lt;ul&gt;&lt;li&gt;&lt;tt&gt;Ctrl-M : &lt;/tt&gt;Toggle mini-mode&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-O : &lt;/tt&gt;Toggle show offline users&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-X : &lt;/tt&gt;Exit&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-H : &lt;/tt&gt;Hide&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-I : &lt;/tt&gt;View the next message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-V : &lt;/tt&gt;View message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-S : &lt;/tt&gt;Send message&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-U : &lt;/tt&gt;Send Url&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-C : &lt;/tt&gt;Send chat request&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-F : &lt;/tt&gt;Send File&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-A : &lt;/tt&gt;Check Auto response&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-P : &lt;/tt&gt;Popup all messages&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-L : &lt;/tt&gt;Redraw user window&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Delete : &lt;/tt&gt;Delete user from current group&lt;/li&gt;&lt;li&gt;&lt;tt&gt;Ctrl-Delete : &lt;/tt&gt;Delete user from contact list&lt;/li&gt;&lt;/ul&gt;&lt;li&gt;Hold control while clicking on close in the function window to remove   the user from your contact list.&lt;/li&gt;&lt;li&gt;Hit Ctrl-Enter from most text entry fields to select &quot;Ok&quot; or &quot;Accept&quot;.   For example in the send tab of the user function window.&lt;/li&gt;&lt;li&gt;Here is the complete list of user % options, which can be used in &lt;b&gt;OnEvent&lt;/b&gt;   parameters, &lt;b&gt;auto responses&lt;/b&gt;, and &lt;b&gt;utilities&lt;/b&gt;:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CMessageViewWidget</name>
    <message>
        <source>%1 from %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 to %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Default</source>
        <translation type="unfinished">デフォルト</translation>
    </message>
    <message>
        <source>Compact</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tiny</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Long</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wide</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CQtLogWindow</name>
    <message>
        <source>&amp;Save</source>
        <translation>セーブ</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>ファイル:%1
が開けませんでした</translation>
    </message>
    <message>
        <source>C&amp;lear</source>
        <translation>クリア</translation>
    </message>
    <message>
        <source>Licq Network Log</source>
        <translation>Licqネットワークログ</translation>
    </message>
</context>
<context>
    <name>CRandomChatDlg</name>
    <message>
        <source>Games</source>
        <translation>ゲーム</translation>
    </message>
    <message>
        <source>Women Seeking Men</source>
        <translation type="obsolete">男性を探す女性</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>20そこそこ</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>30そこそこ</translation>
    </message>
    <message>
        <source>Men Seeking Women</source>
        <translation type="obsolete">女性を探す男性</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>40そこそこ</translation>
    </message>
    <message>
        <source>Random chat search timed out.</source>
        <translation>ランダムチャット検索がタイムアウトしました</translation>
    </message>
    <message>
        <source>Random chat search had an error.</source>
        <translation>ランダムチャット検索にエラーが起こりました</translation>
    </message>
    <message>
        <source>No random chat user found in that group.</source>
        <translation>このグループにはランダムチャットユーザは見付かりませんでした</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>50以上</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>ロマンス</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Searching for Random Chat Partner...</source>
        <translation>ランダムチャットの相手を検索中...</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>探す</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>学生</translation>
    </message>
    <message>
        <source>General</source>
        <translation>一般</translation>
    </message>
    <message>
        <source>Random Chat Search</source>
        <translation>ランダムチャット検索</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CRefuseDlg</name>
    <message>
        <source>Licq %1 Refusal</source>
        <translation>Licq %1 拒否</translation>
    </message>
    <message>
        <source>Refusal message for %1 with </source>
        <translation>%1への拒否メッセージと</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Refuse</source>
        <translation>拒否</translation>
    </message>
</context>
<context>
    <name>CSetRandomChatGroupDlg</name>
    <message>
        <source>&amp;Set</source>
        <translation>セット</translation>
    </message>
    <message>
        <source>done</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>Games</source>
        <translation>ゲーム</translation>
    </message>
    <message>
        <source>Women Seeking Men</source>
        <translation type="obsolete">男性を探す女性</translation>
    </message>
    <message>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>20 Something</source>
        <translation>20そこそこ</translation>
    </message>
    <message>
        <source>30 Something</source>
        <translation>30そこそこ</translation>
    </message>
    <message>
        <source>Men Seeking Women</source>
        <translation type="obsolete">女性を探す男性</translation>
    </message>
    <message>
        <source>40 Something</source>
        <translation>40そこそこ</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>(none)</source>
        <translation>(なし)</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>タイムアウト</translation>
    </message>
    <message>
        <source>Set Random Chat Group</source>
        <translation>ランダムにチャットグループをセット</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>失敗</translation>
    </message>
    <message>
        <source>50 Plus</source>
        <translation>50以上</translation>
    </message>
    <message>
        <source>Romance</source>
        <translation>ロマンス</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Students</source>
        <translation>学生</translation>
    </message>
    <message>
        <source>General</source>
        <translation>一般</translation>
    </message>
    <message>
        <source>Setting Random Chat Group...</source>
        <translation>ランダムチャットグループを設定中...</translation>
    </message>
    <message>
        <source>Seeking Women</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Seeking Men</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CTimeZoneField</name>
    <message>
        <source>Unknown</source>
        <translation type="unfinished">未知</translation>
    </message>
</context>
<context>
    <name>CUserView</name>
    <message>
        <source>S</source>
        <translation>ス</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;u&gt;Auto Response:&lt;/u&gt;</source>
        <translation type="obsolete">自動応答:</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>オンライン</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>オフライン</translation>
    </message>
    <message>
        <source>&lt;br&gt;Licq&amp;nbsp;%1/SSL</source>
        <translation type="obsolete">&lt;br&gt;Licq&amp;nbsp;%1/SSL</translation>
    </message>
    <message>
        <source>&lt;br&gt;Licq&amp;nbsp;%1</source>
        <translation type="obsolete">&lt;br&gt;Licq&amp;nbsp;%1</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;b&gt;Birthday&amp;nbsp;Today!&lt;/b&gt;</source>
        <translation type="obsolete">&lt;br&gt;&lt;b&gt;今日が&amp;nbsp;誕生日!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>%1 Floaty (%2)</source>
        <translation>%1 浮遊中 (%2)</translation>
    </message>
    <message>
        <source>&lt;br&gt;Custom&amp;nbsp;Auto&amp;nbsp;Response</source>
        <translation type="obsolete">&lt;br&gt;自動&amp;nbsp;応答&amp;nbsp;設定</translation>
    </message>
    <message>
        <source>&lt;br&gt;Secure&amp;nbsp;connection</source>
        <translation type="obsolete">&lt;br&gt;安全&amp;nbsp;伝送路</translation>
    </message>
    <message>
        <source>Not In List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> weeks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> week</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> days</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> day</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> hours</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> hour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> minutes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> minute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>0 minutes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Birthday Today!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Typing a message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Phone &amp;quot;Follow Me&amp;quot;: Busy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ICQphone: Available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ICQphone: Busy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File Server: Enabled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Secure connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom Auto Response</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Response:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>P: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>F: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ip: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>O: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Logged In: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Idle: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Local time: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ID: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Awaiting authorization</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CUtilityDlg</name>
    <message>
        <source>GUI</source>
        <translation>GUI</translation>
    </message>
    <message>
        <source>&amp;Run</source>
        <translation>実行</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>Done:</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>Edit:</source>
        <translation>編集:</translation>
    </message>
    <message>
        <source>Edit final command</source>
        <translation>最終コマンドを編集</translation>
    </message>
    <message>
        <source>Command Window</source>
        <translation>コマンドウィンドウ</translation>
    </message>
    <message>
        <source>Licq Utility: %1</source>
        <translation>Licqユーティリティ: %1</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>User Fields</source>
        <translation>ユーザフィールド</translation>
    </message>
    <message>
        <source>Description:</source>
        <translation>説明</translation>
    </message>
    <message>
        <source>Internal</source>
        <translation>内部</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>コマンド:</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Running:</source>
        <translation>実行中:</translation>
    </message>
    <message>
        <source>Failed:</source>
        <translation>失敗:</translation>
    </message>
    <message>
        <source>Terminal</source>
        <translation>ターミナル</translation>
    </message>
    <message>
        <source>Window:</source>
        <translation>ウィンドウ:</translation>
    </message>
</context>
<context>
    <name>ChangePassDlg</name>
    <message>
        <source>&amp;Password:</source>
        <translation type="obsolete">パスワード</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation type="obsolete">再入力</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation type="obsolete">ICQパスワードを入力</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation type="obsolete">再度ICQパスワード入力</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="obsolete">キャンセル</translation>
    </message>
    <message>
        <source>Invalid password, must be 8 characters or less.</source>
        <translation type="obsolete">不正パスワードです。8文字以下。</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation type="obsolete">パスワードが合致しません。もう一度お願いします</translation>
    </message>
</context>
<context>
    <name>ChatDlg</name>
    <message>
        <source>No</source>
        <translation>いいえ</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>はい</translation>
    </message>
    <message>
        <source>Beep</source>
        <translation>ビープ</translation>
    </message>
    <message>
        <source>Bold</source>
        <translation>太字</translation>
    </message>
    <message>
        <source>Chat</source>
        <translation>チャット</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>モード</translation>
    </message>
    <message>
        <source>Toggles Bold font</source>
        <translation>太字消去</translation>
    </message>
    <message>
        <source>ignores user color settings</source>
        <translation type="obsolete">ユーザ色設定を無視</translation>
    </message>
    <message>
        <source>Toggles Italic font</source>
        <translation>斜字体消去</translation>
    </message>
    <message>
        <source>Remote - Waiting for joiners...</source>
        <translation>相手 - 参加待ち...</translation>
    </message>
    <message>
        <source>Licq - Chat</source>
        <translation>Licq - チャット</translation>
    </message>
    <message>
        <source>&amp;Audio</source>
        <translation>オーディオ</translation>
    </message>
    <message>
        <source>Sends a Beep to all recipients</source>
        <translation>全ての受信者にビープを送る</translation>
    </message>
    <message>
        <source>&amp;Pane Mode</source>
        <translation>Paneモード</translation>
    </message>
    <message>
        <source>Italic</source>
        <translation>イタリック</translation>
    </message>
    <message>
        <source>Do you want to save the chat session?</source>
        <translation>チャットセッションをセーブしますか?</translation>
    </message>
    <message>
        <source>Licq - Chat %1</source>
        <translation>Licq - チャット %1</translation>
    </message>
    <message>
        <source>&amp;IRC Mode</source>
        <translation>IRCモード</translation>
    </message>
    <message>
        <source>Changes the foreground color</source>
        <translation>文字色を変える</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>背景色</translation>
    </message>
    <message>
        <source>Local - %1</source>
        <translation>ローカル - %1</translation>
    </message>
    <message>
        <source>/%1.%2.chat</source>
        <translation type="obsolete">/%1.%2.chat</translation>
    </message>
    <message>
        <source>Remote - Not connected</source>
        <translation>相手 - 未接続</translation>
    </message>
    <message>
        <source>&amp;Close Chat</source>
        <translation>チャットを閉じる</translation>
    </message>
    <message>
        <source>Remote - %1</source>
        <translation type="obsolete">相手 - %1</translation>
    </message>
    <message>
        <source>Underline</source>
        <translation>下線</translation>
    </message>
    <message>
        <source>%1 closed connection.</source>
        <translation>%1は接続を閉じました。</translation>
    </message>
    <message>
        <source>Remote - Connecting...</source>
        <translation>相手 - 接続中...</translation>
    </message>
    <message>
        <source>&amp;Save Chat</source>
        <translation>チャットをセーブ</translation>
    </message>
    <message>
        <source>Changes the background color</source>
        <translation>背景色を変える</translation>
    </message>
    <message>
        <source>Foreground color</source>
        <translation>文字色</translation>
    </message>
    <message>
        <source>Ignore user settings</source>
        <translation>ユーザ設定を無視</translation>
    </message>
    <message>
        <source>Ignores user color settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to bind to a port.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to connect to the remote chat.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to create new thread.
See Network Window for details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Toggles Underline font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>StrikeOut</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Toggles StrikeOut font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set Encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>
&lt;--BEEP--&gt;
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&gt; &lt;--BEEP--&gt;
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>/%1.chat</source>
        <translation type="unfinished">/%1.chat</translation>
    </message>
</context>
<context>
    <name>CustomAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation>クリア</translation>
    </message>
    <message>
        <source>&amp;Hints</source>
        <translation>ヒント</translation>
    </message>
    <message>
        <source>Set Custom Auto Response for %1</source>
        <translation>自動応答を %1 とセット</translation>
    </message>
    <message>
        <source>I am currently %1.
You can leave me a message.</source>
        <translation>私は現在 %1
メッセージを残して下さい。</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
</context>
<context>
    <name>EditCategoryDlg</name>
    <message>
        <source>Unspecified</source>
        <translation type="unfinished">無指定</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
</context>
<context>
    <name>EditFileDlg</name>
    <message>
        <source>&amp;Save</source>
        <translation>セーブ</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Failed to open file:
%1</source>
        <translation>ファイル%1 
を開けません</translation>
    </message>
    <message>
        <source>Licq File Editor - %1</source>
        <translation>Licq ファイル編集 - %1</translation>
    </message>
    <message>
        <source>[ Read-Only ]</source>
        <translation>[ 読み専用 ]</translation>
    </message>
</context>
<context>
    <name>EditGrpDlg</name>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>追加</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">編集</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>Edit Name</source>
        <translation>名前編集</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation>グループ</translation>
    </message>
    <message>
        <source>Are you sure you want to remove
the group &apos;%1&apos;?</source>
        <translation>本当にグループ %1
を削除しますか?</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>削除</translation>
    </message>
    <message>
        <source>noname</source>
        <translation>ななしのごんべ</translation>
    </message>
    <message>
        <source>Set Default</source>
        <translation type="obsolete">デフォルトをセット</translation>
    </message>
    <message>
        <source>Default:</source>
        <translation type="obsolete">デフォルト:</translation>
    </message>
    <message>
        <source>The default group to start up in.</source>
        <translation type="obsolete">スタートアップのデフォルトグループ</translation>
    </message>
    <message>
        <source>Edit group name (hit enter to save).</source>
        <translation>名前編集(リターン押してセーブ)</translation>
    </message>
    <message>
        <source>Shift Down</source>
        <translation>下へ</translation>
    </message>
    <message>
        <source>All Users</source>
        <translation type="obsolete">全ユーザ</translation>
    </message>
    <message>
        <source>Licq - Edit Groups</source>
        <translation>Licq - グループ編集</translation>
    </message>
    <message>
        <source>Shift Up</source>
        <translation>上へ</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation type="unfinished">セーブ</translation>
    </message>
    <message>
        <source>Save the name of a group being modified.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>EditPhoneDlg</name>
    <message>
        <source>Description:</source>
        <translation type="unfinished">説明</translation>
    </message>
    <message>
        <source>Home Phone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Work Phone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Private Cellular</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Work Cellular</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Home Fax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Work Fax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wireless Pager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Phone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cellular SMS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Pager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Country:</source>
        <translation type="unfinished">国:</translation>
    </message>
    <message>
        <source>Network #/Area code:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Number:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Extension:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Provider:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E-mail Gateway:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Remove leading 0s from Area Code/Network #</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
    <message>
        <source>@</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please enter a phone number</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Event</name>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>メッセージ</translation>
    </message>
    <message>
        <source>Added to Contact List</source>
        <translation>コンタクトリストに追加されました</translation>
    </message>
    <message>
        <source>Web Panel</source>
        <translation>ウェブパネル</translation>
    </message>
    <message>
        <source>Plugin Event</source>
        <translation>プラグインイベント</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>ファイル転送</translation>
    </message>
    <message>
        <source>Authorization Request</source>
        <translation>承認のリクエスト</translation>
    </message>
    <message>
        <source>Authorization Refused</source>
        <translation>承認は拒否されました</translation>
    </message>
    <message>
        <source> (cancelled)</source>
        <translation type="obsolete"> (キャンセル)</translation>
    </message>
    <message>
        <source>Unknown Event</source>
        <translation>未知のイベント</translation>
    </message>
    <message>
        <source>User Info</source>
        <translation type="obsolete">ユーザ情報</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>チャットリクエスト</translation>
    </message>
    <message>
        <source>Authorization Granted</source>
        <translation>承認は受け付けられました</translation>
    </message>
    <message>
        <source>Email Pager</source>
        <translation>メールページャ</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>コンタクトリスト</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(cancelled)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>System Server Message</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>GPGKeyManager</name>
    <message>
        <source>Licq GPG Key Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPG Passphrase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No passphrase set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Set GPG Passphrase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User Keys</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation type="unfinished">編集</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;qt&gt;Drag&amp;Drop user to add to list.&lt;/qt&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Active</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Key ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation type="unfinished">閉じる</translation>
    </message>
    <message>
        <source>Set Passphrase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not yet implemented. Use licq_gpg.conf.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Remove GPG key</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Do you want to remove the GPG key? The key isn&apos;t deleted from your keyring.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>GPGKeySelect</name>
    <message>
        <source>Select GPG Key for user %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select a GPG key for user %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current key: No key selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current key: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use GPG Encryption</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;No Key</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
</context>
<context>
    <name>Groups</name>
    <message>
        <source>All Users</source>
        <translation type="unfinished">全ユーザ</translation>
    </message>
    <message>
        <source>Online Notify</source>
        <translation type="unfinished">オンライン通知</translation>
    </message>
    <message>
        <source>Visible List</source>
        <translation type="unfinished">可視リスト</translation>
    </message>
    <message>
        <source>Invisible List</source>
        <translation type="unfinished">不可視リスト</translation>
    </message>
    <message>
        <source>Ignore List</source>
        <translation type="unfinished">無視リスト</translation>
    </message>
    <message>
        <source>New Users</source>
        <translation type="unfinished">新ユーザ</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="unfinished">未知</translation>
    </message>
</context>
<context>
    <name>HintsDlg</name>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Licq - Hints</source>
        <translation>Licq - ヒント</translation>
    </message>
</context>
<context>
    <name>IconManager_KDEStyle</name>
    <message>
        <source>&lt;br&gt;1 msg</source>
        <translation>&lt;br&gt;1 メッセージ</translation>
    </message>
    <message>
        <source>&lt;br&gt;%1 msgs</source>
        <translation>&lt;br&gt;%1 メッセージ</translation>
    </message>
    <message>
        <source>&lt;br&gt;Left click - Show main window&lt;br&gt;Middle click - Show next message&lt;br&gt;Right click - System menu</source>
        <translation>&lt;br&gt;左クリック - メインウィンドウを表示&lt;br&gt;真中クリック - 次メッセージを表示&lt;br&gt;右クリック - システムメニュー</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;b&gt;%1 system messages&lt;/b&gt;</source>
        <translation>&lt;br&gt;&lt;br&gt;%1 システムメッセージ&lt;/b&gt;</translation>
    </message>
</context>
<context>
    <name>IconManager_Themed</name>
    <message>
        <source>Unable to load dock theme image
%1</source>
        <translation>ドックテーマファイルを開けません
%1</translation>
    </message>
    <message>
        <source>Unable to load dock theme file
(%1)
:%2</source>
        <translation>ドックテーマファイル(%1)
を開けません
%2</translation>
    </message>
</context>
<context>
    <name>KeyListItem</name>
    <message>
        <source>Yes</source>
        <translation type="unfinished">はい</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="unfinished">いいえ</translation>
    </message>
</context>
<context>
    <name>KeyRequestDlg</name>
    <message>
        <source>&amp;Send</source>
        <translation>送信</translation>
    </message>
    <message>
        <source>Closing secure channel...</source>
        <translation>安全路を閉じています...</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;ForestGreen&quot;&gt;Secure channel established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;ForestGreen&quot;&gt;安全路が確立されました&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Client does not support OpenSSL.
Rebuild Licq with OpenSSL support.</source>
        <translation>クライアントはOpenSSLをサポートしていません
OpenSSLをサポートするよう再構築してください</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;blue&quot;&gt;Secure channel closed.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;blue&quot;&gt;安全路が終了しました&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>Licq - Secure Channel with %1</source>
        <translation>Licq - %1と安全チャンネル</translation>
    </message>
    <message>
        <source>Secure channel is established using SSL
with Diffie-Hellman key exchange and
the TLS version 1 protocol.

</source>
        <translation>SSLを用いて安全路を確保できます
Diffie-Hellman鍵を交換し
TLSバージョン1プロトコルを用います

</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Could not connect to remote client.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;相手と接続できませんでした&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel already established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;安全路はすでに確立されています&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1, however it
has no secure channel support compiled in.
This probably won&apos;t work.</source>
        <translation>相手はLicq %1を使用していますが、
安全路サポート機能をコンパイルしていません
うまく動かないでしょう。</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;yellow&quot;&gt;Secure channel not established.&lt;/font&gt;
</source>
        <translation>&lt;font color=&quot;yellow&quot;&gt;安全路は確立されていません&lt;/font&gt;
</translation>
    </message>
    <message>
        <source>The remote uses Licq %1/SSL.</source>
        <translation>相手は Licq %1/SSLを使用</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Remote client does not support OpenSSL.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;相手はOpenSSLをサポートしていません&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Requesting secure channel...</source>
        <translation>安全路をリクエスト中...</translation>
    </message>
    <message>
        <source>This only works with other Licq clients &gt;= v0.85
The remote doesn&apos;t seem to use such a client.
This might not work.</source>
        <translation>Licqのバージョンが0.85以降のユーザとのみ使用できます
相手ユーザはそのようなクライアントではないようです
なので用いることができないようです</translation>
    </message>
    <message>
        <source>Ready to close channel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ready to request channel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>KeyView</name>
    <message>
        <source>Name</source>
        <translation type="unfinished">名前</translation>
    </message>
</context>
<context>
    <name>MLEditWrap</name>
    <message>
        <source>Allow Tabulations</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MLView</name>
    <message>
        <source>Copy URL</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MsgView</name>
    <message>
        <source>D</source>
        <translation>D</translation>
    </message>
    <message>
        <source>Time</source>
        <translation>時間</translation>
    </message>
    <message>
        <source>Cancelled Event</source>
        <translation>キャンセルされたイベント</translation>
    </message>
    <message>
        <source>Direct</source>
        <translation>直接</translation>
    </message>
    <message>
        <source>Event Type</source>
        <translation>イベントタイプ</translation>
    </message>
    <message>
        <source>Server</source>
        <translation>サーバ</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>緊急</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>オプション</translation>
    </message>
    <message>
        <source>Multiple Recipients</source>
        <translation>複数の受信者</translation>
    </message>
</context>
<context>
    <name>OptionsDlg</name>
    <message>
        <source>&amp;OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Auto</source>
        <translation>自動</translation>
    </message>
    <message>
        <source>Away</source>
        <translation>離席</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>フォント</translation>
    </message>
    <message>
        <source>N/A:</source>
        <translation>忙しい</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>左</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>セーブ</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>none</source>
        <translation>なし</translation>
    </message>
    <message>
        <source>Show Extended Icons</source>
        <translation>拡張アイコンを表示</translation>
    </message>
    <message>
        <source>Flash Events</source>
        <translation type="obsolete">イベントをフラッシュ</translation>
    </message>
    <message>
        <source>Away:</source>
        <translation>離席:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>フォント:</translation>
    </message>
    <message>
        <source>Never</source>
        <translation>永遠</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>右</translation>
    </message>
    <message>
        <source>Text:</source>
        <translation>テキスト:</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>タイトル</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>幅</translation>
    </message>
    <message>
        <source>Parameter for received file transfers</source>
        <translation>受信ファイル転送へのパラメータ</translation>
    </message>
    <message>
        <source>Firewall</source>
        <translation>防火壁</translation>
    </message>
    <message>
        <source>Show Grid Lines</source>
        <translation>グリッド線を表示</translation>
    </message>
    <message>
        <source>Override the skin setting for the frame style of the user window:
   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</source>
        <translation>ユーザウィンドウのフレームスタイルのスキン設定を上書きする:   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)
 + 16 (Plain), 32 (Raised), 48 (Sunken)
 + 240 (Shadow)</translation>
    </message>
    <message>
        <source>Parameter for received chat requests</source>
        <translation>受信チャットリクエストへのパラメータ</translation>
    </message>
    <message>
        <source>Auto Away Messages</source>
        <translation>自動離席メッセージ</translation>
    </message>
    <message>
        <source>Turns on or off the display of headers above each column in the user list</source>
        <translation>ユーザリストのそれぞれのカラムの上のヘッダを自動でオンオフ切替え</translation>
    </message>
    <message>
        <source>Auto Away:</source>
        <translation>離席になるまでの時間:</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;not available&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>自動的に「忙しい」になるまでの時間0だと忙しいにならない</translation>
    </message>
    <message>
        <source>Ignore Mass Messages</source>
        <translation>大量メッセージを無視</translation>
    </message>
    <message>
        <source>Show the &quot;--online--&quot; and &quot;--offline--&quot; bars in the contact list</source>
        <translation>コンタクトリスト中 &quot;--オンライン--&quot; および &quot;--オフライン--&quot; のバーを見せる </translation>
    </message>
    <message>
        <source>Parameter for online notification</source>
        <translation>オンライン通知のパラメータ</translation>
    </message>
    <message>
        <source>TCP port range for incoming connections.</source>
        <translation>受信コネクションのTCPポート番号</translation>
    </message>
    <message>
        <source>Default port to connect to on the server (should be 4000)</source>
        <translation type="obsolete">サーバ接続のデフォルトポート番号(4000であるべき)</translation>
    </message>
    <message>
        <source>Accept Modes</source>
        <translation>受信モード</translation>
    </message>
    <message>
        <source>Online Notify:</source>
        <translation>オンライン通知:</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation>忙しい</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>適用</translation>
    </message>
    <message>
        <source>Determines if new users are automatically added to your list or must first request authorization.</source>
        <translation>新ユーザが自動で自分のリストに追加されるか 承認をリクエストするかを決定</translation>
    </message>
    <message>
        <source>Show the name of the current group in the messages label when there are no new messages</source>
        <translation>新しいメッセージが無い時メッセージラベル中に現在のグループ名を表示する</translation>
    </message>
    <message>
        <source>Ignore Web Panel</source>
        <translation>ウェブパネルを無視</translation>
    </message>
    <message>
        <source>Use Font Styles</source>
        <translation>フォントスタイルを使用</translation>
    </message>
    <message>
        <source>Preset slot:</source>
        <translation>既定値</translation>
    </message>
    <message>
        <source>Licq Options</source>
        <translation>Licqオプション</translation>
    </message>
    <message>
        <source>Show online notify users who are offline even when offline users are hidden.</source>
        <translation>オフラインユーザが隠されている時でもオンライン通知ユーザはオフラインでも表示する</translation>
    </message>
    <message>
        <source>Show Column Headers</source>
        <translation>カラムヘッダを表示</translation>
    </message>
    <message>
        <source>Bold Message Label on Incoming Msg</source>
        <translation>到着メッセージは太字ラベル</translation>
    </message>
    <message>
        <source>Column %1</source>
        <translation>カラム %1</translation>
    </message>
    <message>
        <source>Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf and a shorter 64x48 icon for use in the Gnome/KDE panel.</source>
        <translation>WindowMaker/Afterstepの時は 64x64 を設定しGnome/KDEパネルの時は 64x48 を設定して下さい</translation>
    </message>
    <message>
        <source>Perform OnEvent command in occupied mode</source>
        <translation>用事中の時イベント駆動</translation>
    </message>
    <message>
        <source>Auto Offline:</source>
        <translation>オフラインになるまでの時間</translation>
    </message>
    <message>
        <source>OnEvent in DND</source>
        <translation>邪魔しないで♪の時駆動</translation>
    </message>
    <message>
        <source>OnEvent in N/A</source>
        <translation>忙しいの時駆動</translation>
    </message>
    <message>
        <source>Parameter for received messages</source>
        <translation>受信メッセージへのパラメータ</translation>
    </message>
    <message>
        <source>List of servers to connect to (read-only for now)</source>
        <translation type="obsolete">接続するサーバ(現在は読み込みだけ)</translation>
    </message>
    <message>
        <source>Message Sent:</source>
        <translation>送信メッセージ:</translation>
    </message>
    <message>
        <source>Center</source>
        <translation>中心</translation>
    </message>
    <message>
        <source>Allow scroll bar</source>
        <translation>スクロールバーを許可</translation>
    </message>
    <message>
        <source>OnEvent in Away</source>
        <translation>離席の時駆動</translation>
    </message>
    <message>
        <source>Only urgent events will flash</source>
        <translation type="obsolete">緊急イベントのみフラッシュ</translation>
    </message>
    <message>
        <source>Network</source>
        <translation>ネットワーク</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>フォーマット</translation>
    </message>
    <message>
        <source>The fonts used</source>
        <translation>使用フォント</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically go offline.  Set to &quot;0&quot; to disable.</source>
        <translation>自動的に「オフライン」になるまでの時間0だとオフラインにならない</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation type="obsolete">ロケール</translation>
    </message>
    <message>
        <source>Use Dock Icon</source>
        <translation>ドックアイコンを使用</translation>
    </message>
    <message>
        <source>Chat Request:</source>
        <translation>チャットリクエスト</translation>
    </message>
    <message>
        <source>Online</source>
        <translation>オンライン</translation>
    </message>
    <message>
        <source>Flash Urgent Events</source>
        <translation type="obsolete">緊急イベントをフラッシュ</translation>
    </message>
    <message>
        <source>Server settings</source>
        <translation>サーバ設定</translation>
    </message>
    <message>
        <source>Auto Logon:</source>
        <translation>自動ログオン:</translation>
    </message>
    <message>
        <source>Main Window</source>
        <translation>メインウィンドウ</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>ステータス</translation>
    </message>
    <message>
        <source>Theme:</source>
        <translation>テーマ:</translation>
    </message>
    <message>
        <source>SOCKS5 support is built in but disabled.
To enable it, set the SOCKS5_SERVER
environment variable to &lt;server&gt;:&lt;port&gt;.</source>
        <translation type="obsolete">SOCK5サポート機能はありますが稼働していません
稼働させるには、SOCK5_SERVER環境変数に
&lt;server&gt;:&lt;port&gt;を指定してください。</translation>
    </message>
    <message>
        <source>Always show online notify users</source>
        <translation>オンラインユーザをいつも表示</translation>
    </message>
    <message>
        <source>To enable socks proxy support, install NEC Socks or Dante
then configure the Licq daemon with &quot;--enable-socks5&quot;.</source>
        <translation type="obsolete">プロキシサポートするにはNEC SocksまたはDanteをインストールし
Licqデーモンを&quot;--enable-socks5&quot;を付けてconfigureしてください</translation>
    </message>
    <message>
        <source>default (%1)</source>
        <translation>デフォルト (%1)</translation>
    </message>
    <message>
        <source>Offline</source>
        <translation>オフライン</translation>
    </message>
    <message>
        <source>Port Range:</source>
        <translation>ポートの範囲:</translation>
    </message>
    <message>
        <source>OnEvent</source>
        <translation>イベントの時</translation>
    </message>
    <message>
        <source>Command:</source>
        <translation>コマンド:</translation>
    </message>
    <message>
        <source>Edit Font:</source>
        <translation>フォント編集:</translation>
    </message>
    <message>
        <source>I can receive direct connections</source>
        <translation>直接コネクションを受信できる</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>オプション</translation>
    </message>
    <message>
        <source>Miscellaneous</source>
        <translation>その他いろいろ</translation>
    </message>
    <message>
        <source>All incoming messages automatically open when received, if we are online (or free for chat)</source>
        <translation type="obsolete">もしオンライン(あるいはチャット可能なら)全てのメッセージは到着後 自動的に開く</translation>
    </message>
    <message>
        <source>Perform OnEvent command in do not disturb mode</source>
        <translation>邪魔しないで♪の時イベント駆動</translation>
    </message>
    <message>
        <source>Determines if mass messages are ignored or not.</source>
        <translation>複数メッセージを無視するか決定</translation>
    </message>
    <message>
        <source>Translation:</source>
        <translation type="obsolete">翻訳:</translation>
    </message>
    <message>
        <source>Frame Style: </source>
        <translation>枠のスタイル</translation>
    </message>
    <message>
        <source>The alignment of the column</source>
        <translation>カラムの位置</translation>
    </message>
    <message>
        <source>Make the user window transparent when there is no scroll bar</source>
        <translation>スクロールバーが無いときはユーザウィンドウを透明にする</translation>
    </message>
    <message>
        <source>Default Icon</source>
        <translation>デフォルトアイコン</translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you firstsend an event to them</source>
        <translation type="obsolete">チェックされない時はユーザは自分がメッセージを</translation>
    </message>
    <message>
        <source>The main window will raise on incoming messages</source>
        <translation type="obsolete">メッセージ受信でメインウィンドウが上がる</translation>
    </message>
    <message>
        <source>Check Clipboard For Urls/Files</source>
        <translation>URLとファイルのためのクリップボードチェック</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>パラメータ</translation>
    </message>
    <message>
        <source>Show birthday, invisible, and custom auto response icons to the right of users in the list</source>
        <translation>リスト中のユーザの右側に誕生日、不可視、およびカスタム自動応答アイコンを表示する</translation>
    </message>
    <message>
        <source>Transparent when possible</source>
        <translation>できるだけ透明</translation>
    </message>
    <message>
        <source>Default Auto Response Messages</source>
        <translation>デフォルト自動応答メッセージ</translation>
    </message>
    <message>
        <source>File Transfer:</source>
        <translation>ファイル転送:</translation>
    </message>
    <message>
        <source>OnEvents Enabled</source>
        <translation>イベント駆動を稼働</translation>
    </message>
    <message>
        <source>Paranoia</source>
        <translation>凝った設定</translation>
    </message>
    <message>
        <source>Auto-Raise on Incoming Msg</source>
        <translation>到着メッセージ自動上げ</translation>
    </message>
    <message>
        <source>Terminal:</source>
        <translation>ターミナル:</translation>
    </message>
    <message>
        <source>Automatically log on when first starting up.</source>
        <translation>起動時に自動ログオン</translation>
    </message>
    <message>
        <source>Perform the online notify OnEvent when logging on (this is different from how the Mirabilis client works)</source>
        <translation>ログオンした時オンライン通知駆動イベントを稼働(Mirabilisのクライアント仕様と違います)</translation>
    </message>
    <message>
        <source>Number of minutes of inactivity after which to automatically be marked &quot;away&quot;.  Set to &quot;0&quot; to disable.</source>
        <translation>自動的に「離席」になるまでの時間0だと離席にならない</translation>
    </message>
    <message>
        <source>&lt;p&gt;Command to execute when an event is received.&lt;br&gt;It will be passed the relevant parameters from below.&lt;br&gt;Parameters can contain the following expressions &lt;br&gt; which will be replaced with the relevant information:&lt;/p&gt;</source>
        <translation>&lt;p&gt;イベントを受信したときに実行するコマンド&lt;br&gt;下より適切なパラメータが渡されます &lt;br&gt;パラメータは次の表現を含む事が できます &lt;br&gt; これは適切な情報に置き換え られます:&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Determines if email pager messages are ignored or not.</source>
        <translation>メールページャメッセージを無視するか決定</translation>
    </message>
    <message>
        <source>Perform OnEvent command in not available mode</source>
        <translation>忙しいの時イベント駆動</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation>用事中</translation>
    </message>
    <message>
        <source>Show User Dividers</source>
        <translation>ユーザ間に線を表示</translation>
    </message>
    <message>
        <source>OnEvent in Occupied</source>
        <translation>用事中のとき駆動</translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:
</source>
        <translation type="obsolete">それぞれのカラムに表示されるかを決定するフォーマット文字列
次のパラメータが使用される:
</translation>
    </message>
    <message>
        <source>Parameter for received system messages</source>
        <translation>受信システムメッセージへのパラメータ</translation>
    </message>
    <message>
        <source>Manual &quot;New User&quot; group handling</source>
        <translation>新ユーザのグループを手動で</translation>
    </message>
    <message>
        <source>Parameter for sent messages</source>
        <translation>送信メッセージへのパラメータ</translation>
    </message>
    <message>
        <source>Default Server Port:</source>
        <translation type="obsolete">デフォルトサーバポート:</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation>フォント選択</translation>
    </message>
    <message>
        <source>The message info label will be bold if there are incoming messages</source>
        <translation type="obsolete">メッセージ受信の時メッセージ情報ラベルが太字に</translation>
    </message>
    <message>
        <source>Auto-Popup Incoming Msg</source>
        <translation>到着メッセージ自動ポップアップ</translation>
    </message>
    <message>
        <source>64 x 48 Dock Icon</source>
        <translation>64 x 48ドックアイコン</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Startup</source>
        <translation>スタートアップ</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>ステータス</translation>
    </message>
    <message>
        <source>default</source>
        <translation>デフォルト</translation>
    </message>
    <message>
        <source>Default</source>
        <translation type="obsolete">デフォルト</translation>
    </message>
    <message>
        <source>Use italics and bold in the user list to indicate special characteristics such as online notify and visible list</source>
        <translation>オンライン通知や可視リスト等特別の性質を表すために斜字体と太字を使用する</translation>
    </message>
    <message>
        <source>Use System Background Color</source>
        <translation>システム背景色を使用</translation>
    </message>
    <message>
        <source>Auto N/A:</source>
        <translation>忙しいになるまでの時間</translation>
    </message>
    <message>
        <source>Themed Icon</source>
        <translation>アイコンテーマ</translation>
    </message>
    <message>
        <source>Auto Close Function Window</source>
        <translation>機能ウィンドウ自動閉</translation>
    </message>
    <message>
        <source>Perform OnEvent command in away mode</source>
        <translation>離席の時イベント駆動</translation>
    </message>
    <message>
        <source>Docking</source>
        <translation>ドッキング</translation>
    </message>
    <message>
        <source>Determines if web panel messages are ignored or not.</source>
        <translation>ウェブパネルのメッセージを無視するか決定</translation>
    </message>
    <message>
        <source>Enable running of &quot;Command&quot; when the relevant event occurs.</source>
        <translation>相当するイベントが発生した時「コマンド」を発動</translation>
    </message>
    <message>
        <source>System Msg:</source>
        <translation>システムメッセージ</translation>
    </message>
    <message>
        <source>Show group name if no messages</source>
        <translation>メッセージがないときグループ名を表示</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation>チャットできます</translation>
    </message>
    <message>
        <source>Free For Chat</source>
        <translation>チャットできます</translation>
    </message>
    <message>
        <source>When double-clicking on a user to send a message check for urls/files in the clipboard</source>
        <translation>メッセージを送信するためにユーザをダブルクリックした時クリップボード中のURL/filesをチェックする</translation>
    </message>
    <message>
        <source>Online Notify when Logging On</source>
        <translation>ログオンの時駆動</translation>
    </message>
    <message>
        <source>Extensions</source>
        <translation>拡張</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation>邪魔しないで♪</translation>
    </message>
    <message>
        <source>All incoming events will flash</source>
        <translation type="obsolete">全ての受信イベントがフラッシュする</translation>
    </message>
    <message>
        <source>Servers:</source>
        <translation type="obsolete">サーバ</translation>
    </message>
    <message>
        <source>Url Viewer:</source>
        <translation>URLビューア:</translation>
    </message>
    <message>
        <source>Message:</source>
        <translation>メッセージ:</translation>
    </message>
    <message>
        <source>Draw boxes around each square in the user list</source>
        <translation>ユーザリストの端にボックスを描く</translation>
    </message>
    <message>
        <source>Sets the default behavior for auto closing the user function window after a succesful event</source>
        <translation type="obsolete">自動で閉じる際のデフォルト挙動を設定イベント後のユーザ機能ウィンドウ</translation>
    </message>
    <message>
        <source>Column Configuration</source>
        <translation>カラム設定</translation>
    </message>
    <message>
        <source>Invisible</source>
        <translation>不可視</translation>
    </message>
    <message>
        <source>General</source>
        <translation>一般</translation>
    </message>
    <message>
        <source>Ignore Email Pager</source>
        <translation>メールページャを無視</translation>
    </message>
    <message>
        <source>Controls whether or not the dockable icon should be displayed.</source>
        <translation>ドック可能アイコン表示かどうかをコントロール</translation>
    </message>
    <message>
        <source>Sort Online Users by Status</source>
        <translation type="obsolete">ステータスでオンラインユーザをソート</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>コンタクトリスト</translation>
    </message>
    <message>
        <source>The width of the column</source>
        <translation>カラムの幅</translation>
    </message>
    <message>
        <source>Previous Message</source>
        <translation>前のメッセージ</translation>
    </message>
    <message>
        <source>Sort all online users by their actual status</source>
        <translation type="obsolete">実際のステータスでオンラインユーザをソート</translation>
    </message>
    <message>
        <source>Select a font from the system list</source>
        <translation>システムリストからフォントを選択</translation>
    </message>
    <message>
        <source>The string which will appear in the list box column header</source>
        <translation>リストボックスのカラムヘッダに表示される文字列</translation>
    </message>
    <message>
        <source>Parameter for received URLs</source>
        <translation>受信URLへのパラメータ</translation>
    </message>
    <message>
        <source>The command to run to view a URL.  Will be passed the URL as a parameter.</source>
        <translation>URLを表示するコマンド。URLをパラメータとして渡します</translation>
    </message>
    <message>
        <source>Allow the vertical scroll bar in the user list</source>
        <translation>ユーザリストに垂直スクロールバーを許可</translation>
    </message>
    <message>
        <source>The command to run to start your terminal program.</source>
        <translation>ターミナルプログラムを動かすコマンド</translation>
    </message>
    <message>
        <source>Sets which translation table should be used for translating characters.</source>
        <translation type="obsolete">文字翻訳に使用する翻訳テーブルを設定</translation>
    </message>
    <message>
        <source>Font used in message editor etc.</source>
        <translation>メッセージエディタ等で用いられるフォント</translation>
    </message>
    <message>
        <source>Ignore New Users</source>
        <translation>新ユーザを無視</translation>
    </message>
    <message>
        <source>Alignment</source>
        <translation>配置</translation>
    </message>
    <message>
        <source>Auto close the user function window after a succesful event</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open all incoming messages automatically when received if we are online (or free for chat)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Raise the main window on incoming messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Start Hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Start main window hidden. Only the dock icon will be visible.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show the message info label in bold font if there are incoming messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If not checked, a user will be automatically removed from &quot;New User&quot; group when you first send an event to them</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Position the Reply Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Position a new reply window just underneath the message view window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto send through server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Automatically send messages through the server if direct connection fails</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Allow dragging main window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Lets you drag around the main window with your mouse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Chatmode Messageview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show the current chat history in Send Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Localization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Default Encoding:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sets which default encoding should be used for newly added contacts.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>System default (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show all encodings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ICQ Server:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ICQ Server Port:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source><byte value="x9"/>to</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use proxy server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>HTTPS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy Server:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy Server Port:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use authorization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="unfinished">パスワード:</translation>
    </message>
    <message>
        <source>Use server side contact list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Store your contacts on the server so they are accessible from different locations and/or programs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Blink All Events</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>All incoming events will blink</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Blink Urgent Events</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Only urgent events will blink</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hot key: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hotkey pops up the next pending message
Enter the hotkey literally, like &quot;shift+f10&quot;, &quot;none&quot; for disabling
changes here require a Restart to take effect!
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Popup info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Email</source>
        <translation type="unfinished">メール</translation>
    </message>
    <message>
        <source>Phone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cellular</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>IP</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Last online</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Online Time</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Idle Time</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Message Display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Flash Taskbar on Incoming Msg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Flash the Taskbar on incoming messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tabbed Chatting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use tabs in Send Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show recent messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show the last 5 messages when a Send Window is opened</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send typing notifications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Send a notification to the user so they can see when you are typing a message to them</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sticky Main Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Makes the Main window visible on all desktops</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sticky Message Window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Makes the Message window visible on all desktops</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Small Icon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Uses the freedesktop.org standard to dock a small icon into the window manager.  Works with many different window managers.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>I am behind a firewall</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Reconnect after Uin clash</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq can reconnect you when you got disconnected because your Uin was used from another location. Check this if you want Licq to reconnect automatically.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Additional &amp;sorting:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>status + last event</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>status + new messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;none:&lt;/b&gt; - Don&apos;t sort online users by Status&lt;br&gt;
&lt;b&gt;status&lt;/b&gt; - Sort online users by status&lt;br&gt;
&lt;b&gt;status + last event&lt;/b&gt; - Sort online users by status and by last event&lt;br&gt;
&lt;b&gt;status + new messages&lt;/b&gt; - Sort online users by status and number of new messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show user display picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show the user&apos;s display picture instead of a status icon, if the user is online and has a display picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Protocol ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Update</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Update Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Automatically update users&apos; server stored information.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Update Info Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone Book and Picture.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto Update Status Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Automatically update users&apos; Phone &quot;Follow Me&quot;, File Server and ICQphone status.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Style:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Insert Horizontal Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Colors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Message Received:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History Received:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History Sent:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Typing Notification Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Single line chat mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>In single line chat mode you send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alias</source>
        <translation type="unfinished">ニックネーム</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">名前</translation>
    </message>
    <message>
        <source>Date Format:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Notice:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is a sent message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Have you gone to the Licq IRC Channel?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No, where is it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>#Licq on irc.freenode.net</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cool, I&apos;ll see you there :)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>We&apos;ll be waiting!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Marge has left the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is a received message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use double return</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hitting Return twice will be used instead of Ctrl+Return to send messages and close input dialogs. Multiple new lines can be inserted with Ctrl+Return.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show Join/Left Notices</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show a notice in the chat window when a user joins or leaves the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Local time</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Chat Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Insert Vertical Spacing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Insert extra space between messages.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Insert a line between each message.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;Available custom date format variables.&lt;/p&gt;
&lt;table&gt;
&lt;tr&gt;&lt;th&gt;Expression&lt;/th&gt;&lt;th&gt;Output&lt;/th&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;d&lt;/td&gt;&lt;td&gt;the day as number without a leading zero (1-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dd&lt;/td&gt;&lt;td&gt;the day as number with a leading zero (01-31)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ddd&lt;/td&gt;&lt;td&gt;the abbreviated localized day name (e.g. &apos;Mon&apos;..&apos;Sun&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;dddd&lt;/td&gt;&lt;td&gt;the long localized day name (e.g. &apos;Monday&apos;..&apos;Sunday&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;M&lt;/td&gt;&lt;td&gt;the month as number without a leading zero (1-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MM&lt;/td&gt;&lt;td&gt;the month as number with a leading zero (01-12)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMM&lt;/td&gt;&lt;td&gt;the abbreviated localized month name (e.g. &apos;Jan&apos;..&apos;Dec&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;MMMM&lt;/td&gt;&lt;td&gt;the long localized month name (e.g. &apos;January&apos;..&apos;December&apos;)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yy&lt;/td&gt;&lt;td&gt;the year as two digit number (00-99)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;yyyy&lt;/td&gt;&lt;td&gt;the year as four digit number (1752-8000)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td colspan=2&gt;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;h&lt;/td&gt;&lt;td&gt;the hour without a leading zero (0..23 or 1..12 if AM/PM display)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;hh&lt;/td&gt;&lt;td&gt;the hour with a leading zero (00..23 or 01..12 if AM/PM display)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;m&lt;/td&gt;&lt;td&gt;the minute without a leading zero (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;mm&lt;/td&gt;&lt;td&gt;the minute with a leading zero (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;s&lt;/td&gt;&lt;td&gt;the second without a leading zero (0..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ss&lt;/td&gt;&lt;td&gt;the second with a leading zero (00..59)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;z&lt;/td&gt;&lt;td&gt;the millisecond without leading zero (0..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;zzz&lt;/td&gt;&lt;td&gt;the millisecond with leading zero (000..999)&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;AP&lt;/td&gt;&lt;td&gt;use AM/PM display. AP will be replaced by either &apos;AM&apos; or &apos;PM&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;tr&gt;&lt;td&gt;ap&lt;/td&gt;&lt;td&gt;use am/pm display. ap will be replaced by either &apos;am&apos; or &apos;pm&apos;&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The format string used to define what will appear in each column.
The following parameters can be used:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>OwnerEditDlg</name>
    <message>
        <source>Edit Account</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="unfinished">パスワード:</translation>
    </message>
    <message>
        <source>Protocol:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Currently only one account per protocol is supported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
</context>
<context>
    <name>OwnerManagerDlg</name>
    <message>
        <source>Licq - Account Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Register</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Modify</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>D&amp;elete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>From the Account Manager dialog you are able to add and register your accounts.
Currently, only one account per protocol is supported, but this will be changed in future versions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You are currently registered as
UIN (User ID): %1
Base Directory: %2
Rerun licq with the -b option to select a new
base directory and then register a new user.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Successfully registered, your user identification
number (UIN) is %1.
Now set your personal information.</source>
        <translation type="unfinished">登録に成功しました。あなたのICQ番号(UIN)
は %1 です。
次にあなたの個人情報を設定してください。</translation>
    </message>
    <message>
        <source>Registration failed.  See network window for details.</source>
        <translation type="unfinished">登録に失敗しました。詳細はネットワークウィンドウを御覧ください</translation>
    </message>
</context>
<context>
    <name>OwnerView</name>
    <message>
        <source>User ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Protocol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(Invalid ID)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Invalid Protocol</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>PasswordDlg</name>
    <message>
        <source>error</source>
        <translation type="obsolete">エラー</translation>
    </message>
    <message>
        <source>Set Password [Setting...</source>
        <translation type="obsolete">パスワード設定 [設定中...</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation type="obsolete">タイムアウト</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation type="obsolete">再度ICQパスワード入力</translation>
    </message>
    <message>
        <source>failed</source>
        <translation type="obsolete">失敗</translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation type="obsolete">パスワードが合致しません。もう一度お願いします</translation>
    </message>
    <message>
        <source>Invalid password, must be 8 characters or less.</source>
        <translation type="obsolete">不正パスワードです。8文字以下。</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="obsolete">パスワード</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="obsolete">キャンセル</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="obsolete">パスワード</translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation type="obsolete">再入力</translation>
    </message>
    <message>
        <source>Set Password</source>
        <translation type="obsolete">パスワードをセット</translation>
    </message>
    <message>
        <source>ICQ Password [Setting...]</source>
        <translation type="obsolete">ICQパスワード [設定中...]</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation type="obsolete">ICQパスワードを入力</translation>
    </message>
    <message>
        <source>Licq - Set Password</source>
        <translation type="obsolete">Licq - パスワードをセット</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation type="obsolete">更新</translation>
    </message>
</context>
<context>
    <name>PluginDlg</name>
    <message>
        <source>Id</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>完了</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>読み込み</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <source>Plugin %1 has no configuration file</source>
        <translation>プラグイン%1は設定ファイルが存在しません</translation>
    </message>
    <message>
        <source>Enable</source>
        <translation>使用する</translation>
    </message>
    <message>
        <source>Configure</source>
        <translation type="obsolete">設定</translation>
    </message>
    <message>
        <source>Loaded</source>
        <translation type="obsolete">読み込み済</translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="obsolete">ステータス</translation>
    </message>
    <message>
        <source>Unload</source>
        <translation type="obsolete">アンロード</translation>
    </message>
    <message>
        <source>Licq Plugin %1 %2
</source>
        <translation>Licq プラグイン %1 %2
</translation>
    </message>
    <message>
        <source>Available</source>
        <translation type="obsolete">利用可能</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>リフレッシュ</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>詳細</translation>
    </message>
    <message>
        <source>Details</source>
        <translation type="obsolete">詳細</translation>
    </message>
    <message>
        <source>Disable</source>
        <translation type="obsolete">使用しない</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>バージョン</translation>
    </message>
    <message>
        <source>Standard Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Protocol Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(Unloaded)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation type="obsolete">いいえ</translation>
    </message>
    <message>
        <source>Dir</source>
        <translation type="obsolete">Dir</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation type="obsolete">はい</translation>
    </message>
    <message>
        <source>Back</source>
        <translation type="obsolete">戻る</translation>
    </message>
    <message>
        <source>Date</source>
        <translation type="obsolete">日にち</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="obsolete">ファイル</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">名前</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="obsolete">開く</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="obsolete">セーブ</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="obsolete">サイズ</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation type="obsolete">ソート</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">タイプ</translation>
    </message>
    <message>
        <source>&amp;Open</source>
        <translation type="obsolete">開く</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation type="obsolete">セーブ</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation type="obsolete">エラー</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">エラー</translation>
    </message>
    <message>
        <source>Sort by &amp;Size</source>
        <translation type="obsolete">サイズでソート</translation>
    </message>
    <message>
        <source>Sort by &amp;Date</source>
        <translation type="obsolete">日付でソート</translation>
    </message>
    <message>
        <source>Sort by &amp;Name</source>
        <translation type="obsolete">名前でソート</translation>
    </message>
    <message>
        <source>New Folder 1</source>
        <translation type="obsolete">新フォルダ1</translation>
    </message>
    <message>
        <source>&lt;qt&gt;Do you really want to delete %1 &quot;%2&quot;?&lt;/qt&gt;</source>
        <translation type="obsolete">&lt;qt&gt;本当に%1 &quot;%2&quot;を消しますか?&lt;/qt&gt;</translation>
    </message>
    <message>
        <source>the directory</source>
        <translation type="obsolete">ディレクトリ</translation>
    </message>
    <message>
        <source>File &amp;type:</source>
        <translation type="obsolete">ファイルタイプ:</translation>
    </message>
    <message>
        <source>File &amp;name:</source>
        <translation type="obsolete">ファイル名:</translation>
    </message>
    <message>
        <source>Delete %1</source>
        <translation type="obsolete">%1を消去</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">キャンセル</translation>
    </message>
    <message>
        <source>R&amp;eload</source>
        <translation type="obsolete">リロード</translation>
    </message>
    <message>
        <source>New Folder</source>
        <translation type="obsolete">新フォルダ</translation>
    </message>
    <message>
        <source>&amp;Unsorted</source>
        <translation type="obsolete">仕分けなし</translation>
    </message>
    <message>
        <source>Look &amp;in:</source>
        <translation type="obsolete">見る</translation>
    </message>
    <message>
        <source>Preview File Contents</source>
        <translation type="obsolete">ファイル内容をプレビュー</translation>
    </message>
    <message>
        <source>New Folder %1</source>
        <translation type="obsolete">新フォルダ %1</translation>
    </message>
    <message>
        <source>Read-write</source>
        <translation type="obsolete">読み-書き</translation>
    </message>
    <message>
        <source>Read-only</source>
        <translation type="obsolete">読み専用</translation>
    </message>
    <message>
        <source>Find Directory</source>
        <translation type="obsolete">ディレクトリ見付ける</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation type="obsolete">属性</translation>
    </message>
    <message>
        <source>Show &amp;hidden files</source>
        <translation type="obsolete">隠しファイルも表示</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation type="obsolete">セーブ</translation>
    </message>
    <message>
        <source>Inaccessible</source>
        <translation type="obsolete">アクセス不可</translation>
    </message>
    <message>
        <source>%1
File not found.
Check path and filename.</source>
        <translation type="obsolete">%1
ファイルは見付かりませんでした
パス名とファイル名をチェックしてください</translation>
    </message>
    <message>
        <source>List View</source>
        <translation type="obsolete">リストビュー</translation>
    </message>
    <message>
        <source>Special</source>
        <translation type="obsolete">特別</translation>
    </message>
    <message>
        <source>Write-only</source>
        <translation type="obsolete">書き専用</translation>
    </message>
    <message>
        <source>the symlink</source>
        <translation type="obsolete">the symlink</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation type="obsolete">消去</translation>
    </message>
    <message>
        <source>All files (*)</source>
        <translation type="obsolete">全ファイル (*)</translation>
    </message>
    <message>
        <source>Directories</source>
        <translation type="obsolete">ディレクトリ</translation>
    </message>
    <message>
        <source>Symlink to Special</source>
        <translation type="obsolete">特別へsymlink</translation>
    </message>
    <message>
        <source>&amp;Rename</source>
        <translation type="obsolete">リネーム</translation>
    </message>
    <message>
        <source>One directory up</source>
        <translation type="obsolete">一つ上のディレクトリへ</translation>
    </message>
    <message>
        <source>Preview File Info</source>
        <translation type="obsolete">ファイル情報プレビュー</translation>
    </message>
    <message>
        <source>the file</source>
        <translation type="obsolete">ファイル</translation>
    </message>
    <message>
        <source>Create New Folder</source>
        <translation type="obsolete">新フォルダ作成</translation>
    </message>
    <message>
        <source>Symlink to File</source>
        <translation type="obsolete">ファイルへsymlink</translation>
    </message>
    <message>
        <source>Symlink to Directory</source>
        <translation type="obsolete">ディレクトリへsymlink</translation>
    </message>
    <message>
        <source>Detail View</source>
        <translation type="obsolete">詳細ビュー</translation>
    </message>
</context>
<context>
    <name>QFontDialog</name>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>&amp;Font</source>
        <translation type="obsolete">フォント</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation type="obsolete">サイズ</translation>
    </message>
    <message>
        <source>Apply</source>
        <translation type="obsolete">適用</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="obsolete">閉じる</translation>
    </message>
    <message>
        <source>&amp;Color</source>
        <translation type="obsolete">色</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">キャンセル</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation type="obsolete">サンプル</translation>
    </message>
    <message>
        <source>Font st&amp;yle</source>
        <translation type="obsolete">フォントスタイル</translation>
    </message>
    <message>
        <source>Scr&amp;ipt</source>
        <translation type="obsolete">スクリプト</translation>
    </message>
    <message>
        <source>Select Font</source>
        <translation type="obsolete">フォント選択</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation type="obsolete">下線</translation>
    </message>
    <message>
        <source>Effects</source>
        <translation type="obsolete">効果</translation>
    </message>
    <message>
        <source>Stri&amp;keout</source>
        <translation type="obsolete">線を引っ張る</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <source>Licq Error</source>
        <translation type="obsolete">Licqエラー</translation>
    </message>
    <message>
        <source>Licq Warning</source>
        <translation type="obsolete">Licq警告</translation>
    </message>
    <message>
        <source>Licq Question</source>
        <translation>Licq質問</translation>
    </message>
    <message>
        <source>Licq Information</source>
        <translation type="obsolete">Licq情報</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="unfinished">はい</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="unfinished">いいえ</translation>
    </message>
</context>
<context>
    <name>QString</name>
    <message>
        <source>Failed to open file:
%1</source>
        <translation type="obsolete">ファイル
%1を開けませんでした</translation>
    </message>
</context>
<context>
    <name>QWizard</name>
    <message>
        <source>Help</source>
        <translation type="obsolete">ヘルプ</translation>
    </message>
    <message>
        <source>&lt; Back</source>
        <translation type="obsolete">&lt; 戻る</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">キャンセル</translation>
    </message>
    <message>
        <source>Finish</source>
        <translation type="obsolete">終了</translation>
    </message>
    <message>
        <source>Next &gt;</source>
        <translation type="obsolete">次 &gt;</translation>
    </message>
</context>
<context>
    <name>RegisterUserDlg</name>
    <message>
        <source>Uin:</source>
        <translation type="obsolete">Uin:</translation>
    </message>
    <message>
        <source>Please enter your password in both input fields.</source>
        <translation>両方のフィールドにパスワードを入力してください。</translation>
    </message>
    <message>
        <source>Registered succesfully.  Now log on and update your personal info.</source>
        <translation type="obsolete">登録に成功しました。ログオンし個人情報をアップデートしてください。</translation>
    </message>
    <message>
        <source>You need to enter a valid UIN when you try to register an existing user. </source>
        <translation type="obsolete">既存ユーザ登録をするには 正しいUINを入力する必要があります。 </translation>
    </message>
    <message>
        <source>Now please click &apos;Finish&apos; to start the registration process.</source>
        <translation>「終了」をクリックして 登録プロセスを開始してください。</translation>
    </message>
    <message>
        <source>User Registration in Progress...</source>
        <translation type="obsolete">ユーザ登録進行中...</translation>
    </message>
    <message>
        <source>Now please press the &apos;Back&apos; button and try again.</source>
        <translation>「戻る」を押して 再度入力してください。</translation>
    </message>
    <message>
        <source>&amp;Register Existing User</source>
        <translation type="obsolete">既存ユーザ登録</translation>
    </message>
    <message>
        <source>The passwords don&apos;t seem to match.</source>
        <translation>入力パスワードが合致しません</translation>
    </message>
    <message>
        <source>UIN Registration - Step 2</source>
        <translation type="obsolete">UIN登録 - ステップ2</translation>
    </message>
    <message>
        <source>UIN Registration - Step 3</source>
        <translation type="obsolete">UIN登録 - ステップ3</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation>パスワード:</translation>
    </message>
    <message>
        <source>Invalid UIN.  Try again.</source>
        <translation type="obsolete">UINが正しくありません。もう一度お願いします</translation>
    </message>
    <message>
        <source>Verify:</source>
        <translation>再入力:</translation>
    </message>
    <message>
        <source>Licq User Registration</source>
        <translation type="obsolete">Licq ユーザ登録</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here, or configure Licq to use an existing UIN.

If you are registering a new uin, choose a password and click &quot;Finish&quot;.
If you already have a uin, then toggle &quot;Register Existing User&quot;,
enter your uin and your password, and click &quot;OK&quot;

Press &quot;Next&quot; to proceed.</source>
        <translation type="obsolete">登録ウィザードへようこそ!

新規ユーザとして登録するか既存ユーザのUINを 設定できます。

新UINを登録するときはパスワードを選んで 「終了」をクリックしてください。
UINをお持ちの方は「既存ユーザ登録」をチェックし
あなたのUINとパスワードを入力し「OK」をクリックしてください。

「次へ」を押して進んで下さい</translation>
    </message>
    <message>
        <source>UIN Registration</source>
        <translation type="obsolete">UIN登録</translation>
    </message>
    <message>
        <source>Welcome to the Registration Wizard.

You can register a new user here.

Press &quot;Next&quot; to proceed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Account Registration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter a password to protect your account.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Remember Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Account Registration - Step 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Account Registration - Step 3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq Account Registration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Account Registration in Progress...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Account registration has been successfuly completed.
Your new user id is %1.
You are now being automatically logged on.
Click OK to edit your personal details.
After you are online, you can send your personal details to the server.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ReqAuthDlg</name>
    <message>
        <source>Licq - Request Authorization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request authorization from (UIN):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
</context>
<context>
    <name>SearchUserDlg</name>
    <message>
        <source>Male</source>
        <translation>男性</translation>
    </message>
    <message>
        <source>Reset Search</source>
        <translation>検索リセット</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>&amp;Uin#</source>
        <translation type="obsolete">UIN</translation>
    </message>
    <message>
        <source>City:</source>
        <translation>都市:</translation>
    </message>
    <message>
        <source>UIN#:</source>
        <translation>UIN#</translation>
    </message>
    <message>
        <source>Company Name:</source>
        <translation>会社名:</translation>
    </message>
    <message>
        <source>Enter search parameters and select &apos;Search&apos;</source>
        <translation>パラメタを入力して「検索」をクリックしてください</translation>
    </message>
    <message>
        <source>Company Position:</source>
        <translation>会社での役職:</translation>
    </message>
    <message>
        <source>Searching (this can take awhile)...</source>
        <translation>検索中 (しばらくかかることもあります)...</translation>
    </message>
    <message>
        <source>&amp;Email</source>
        <translation type="obsolete">メール</translation>
    </message>
    <message>
        <source>%1 more users found. Narrow search.</source>
        <translation>%1もっとユーザが見付かりました。局所サーチ</translation>
    </message>
    <message>
        <source>More users found. Narrow search.</source>
        <translation>大勢のユーザが見付かりました。範囲を狭めてください。</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>ニックネーム</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>女性</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation>言語:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>州:</translation>
    </message>
    <message>
        <source>First Name:</source>
        <translation>名:</translation>
    </message>
    <message>
        <source>Last Name:</source>
        <translation>姓:</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>国:</translation>
    </message>
    <message>
        <source>Search failed.</source>
        <translation>検索失敗</translation>
    </message>
    <message>
        <source>Company Department:</source>
        <translation>会社の部署名:</translation>
    </message>
    <message>
        <source>Return Online Users Only</source>
        <translation>オンラインユーザのみ</translation>
    </message>
    <message>
        <source>A&amp;lert User</source>
        <translation>ユーザに通知</translation>
    </message>
    <message>
        <source>&amp;Add %1 Users</source>
        <translation>%1ユーザを追加</translation>
    </message>
    <message>
        <source>Email Address:</source>
        <translation>メールアドレス:</translation>
    </message>
    <message>
        <source>&amp;Add User</source>
        <translation>ユーザ追加</translation>
    </message>
    <message>
        <source>&amp;Whitepages</source>
        <translation>ホワイトページ</translation>
    </message>
    <message>
        <source>&amp;Search</source>
        <translation>検索</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>無指定</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>性別:</translation>
    </message>
    <message>
        <source>Minimum Age:</source>
        <translation type="obsolete">最小年齢</translation>
    </message>
    <message>
        <source>Maximum Age:</source>
        <translation type="obsolete">最高年齢</translation>
    </message>
    <message>
        <source>Licq - User Search</source>
        <translation>Licq - ユーザ検索</translation>
    </message>
    <message>
        <source>Age Range:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>18 - 22</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>23 - 29</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>30 - 39</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>40 - 49</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>50 - 59</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>60+</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Keyword:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;UIN#</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>View &amp;Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search complete.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SearchUserView</name>
    <message>
        <source>UIN</source>
        <translation>UIN</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <source>Alias</source>
        <translation>ニックネーム</translation>
    </message>
    <message>
        <source>Email</source>
        <translation>メール</translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="unfinished">ステータス</translation>
    </message>
    <message>
        <source>Sex &amp; Age</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Authorize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Offline</source>
        <translation type="unfinished">オフライン</translation>
    </message>
    <message>
        <source>Online</source>
        <translation type="unfinished">オンライン</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="unfinished">未知</translation>
    </message>
    <message>
        <source>F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>M</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No</source>
        <translation type="unfinished">いいえ</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="unfinished">はい</translation>
    </message>
</context>
<context>
    <name>SecurityDlg</name>
    <message>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...]</source>
        <translation>ICQセキュリティオプション [設定中...</translation>
    </message>
    <message>
        <source>ICQ Security Options</source>
        <translation>ICQセキュリティオプション</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change the settings.</source>
        <translation>設定を変更するにはICQネットワーク
に接続する必要があります</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>タイムアウト</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>オプション</translation>
    </message>
    <message>
        <source>Web presence allows users to see if you are online through your web indicator.</source>
        <translation type="obsolete">ウェブ存在はウェブインジケータからあなたがオンラインかどうかを知る手段を提供します</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>失敗</translation>
    </message>
    <message>
        <source>Hiding ip stops users from seeing your ip.</source>
        <translation type="obsolete">IPを隠すことで他人にIPを知られないで済む</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>承認が必要</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>更新</translation>
    </message>
    <message>
        <source>Web Presence</source>
        <translation>ウェブ上での存在</translation>
    </message>
    <message>
        <source>ICQ Security Options [Setting...</source>
        <translation>ICQセキュリティオプション [設定中...</translation>
    </message>
    <message>
        <source>Determines whether regular ICQ clients require your authorization to add you to their contact list.</source>
        <translation>通常のICQクライアントがあなたをコンタクトリストに加える際にあなたの承認を得る必要があるかどうかを決定</translation>
    </message>
    <message>
        <source>Hide IP</source>
        <translation>IP隠す</translation>
    </message>
    <message>
        <source>Password/UIN settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Uin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="unfinished">パスワード</translation>
    </message>
    <message>
        <source>Enter your ICQ password here.</source>
        <translation type="unfinished">ICQパスワードを入力</translation>
    </message>
    <message>
        <source>Verify your ICQ password here.</source>
        <translation type="unfinished">再度ICQパスワード入力</translation>
    </message>
    <message>
        <source>&amp;Local changes only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If checked, password/UIN changes will apply only on your local computer.  Useful if your password is incorrectly saved in Licq.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Web Presence allows users to see if you are online through your web indicator.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hide IP stops users from seeing your IP address. It doesn&apos;t guarantee it will be hidden though.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Invalid password, must be between 1 and 8 characters.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Passwords do not match, try again.</source>
        <translation type="unfinished">パスワードが合致しません。もう一度お願いします</translation>
    </message>
    <message>
        <source>Setting security options failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Changing password failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Timeout while setting security options.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Timeout while changing password.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Internal error while setting security options.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Internal error while changing password.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter the UIN which you want to use.  Only available if &quot;Local changes only&quot; is checked.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Verify:</source>
        <translation type="unfinished">再入力</translation>
    </message>
</context>
<context>
    <name>ShowAwayMsgDlg</name>
    <message>
        <source>&amp;Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>タイムアウト</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>失敗</translation>
    </message>
    <message>
        <source>&amp;Show Again</source>
        <translation>再び表示</translation>
    </message>
    <message>
        <source>%1 Response for %2</source>
        <translation>%2への%1応答</translation>
    </message>
    <message>
        <source>refused</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SkinBrowserDlg</name>
    <message>
        <source>Name</source>
        <translation type="obsolete">名前</translation>
    </message>
    <message>
        <source>&amp;Done</source>
        <translation type="obsolete">完了</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation type="obsolete">編集</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>Icons</source>
        <translation type="obsolete">アイコン</translation>
    </message>
    <message>
        <source>Skins</source>
        <translation type="obsolete">スキン</translation>
    </message>
    <message>
        <source>&amp;Apply</source>
        <translation>適用</translation>
    </message>
    <message>
        <source>A&amp;pply</source>
        <translation type="obsolete">適用</translation>
    </message>
    <message>
        <source>Licq Skin Browser</source>
        <translation>LICQスキンブラウザ</translation>
    </message>
    <message>
        <source>Skin selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>S&amp;kins:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available skins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Icons:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available icon sets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E&amp;xtended Icons:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available extended icon sets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use this combo box to select one of the available emoticon icon sets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Skin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Icons:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Extended Icons:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Emoticons:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Edit Skin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
    <message>
        <source>Unable to open icons file
%1
Iconset &apos;%2&apos; has been disabled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1
Extended Iconset &apos;%2&apos; has been disabled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to open icons file
%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to open extended icons file
%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E&amp;moticons:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Status</name>
    <message>
        <source>Offline</source>
        <translation type="unfinished">オフライン</translation>
    </message>
    <message>
        <source>Online</source>
        <translation type="unfinished">オンライン</translation>
    </message>
    <message>
        <source>Away</source>
        <translation type="unfinished">離席</translation>
    </message>
    <message>
        <source>Do Not Disturb</source>
        <translation type="unfinished">邪魔しないで♪</translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation type="unfinished">忙しい</translation>
    </message>
    <message>
        <source>Occupied</source>
        <translation type="unfinished">用事中</translation>
    </message>
    <message>
        <source>Free for Chat</source>
        <translation type="unfinished">チャットできます</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="unfinished">未知</translation>
    </message>
    <message>
        <source>Off</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>On</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>DND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>N/A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Occ</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>FFC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>???</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserCodec</name>
    <message>
        <source>Unicode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Arabic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Baltic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Central European</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Chinese</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Chinese Traditional</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cyrillic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Esperanto</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Greek</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hebrew</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Japanese</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Korean</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Western European</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tamil</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Thai</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Turkish</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ukrainian</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unicode-16</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserEventCommon</name>
    <message>
        <source>Time:</source>
        <translation>時間</translation>
    </message>
    <message>
        <source>Show User Info</source>
        <translation>ユーザ情報を表示</translation>
    </message>
    <message>
        <source>Secure channel information</source>
        <translation type="obsolete">安全チャンネル情報</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>ステータス</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <source>Show User History</source>
        <translation>ユーザ履歴を表示</translation>
    </message>
    <message>
        <source>Open / Close secure channel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Change user text encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to load encoding &lt;b&gt;%1&lt;/b&gt;. Message contents may appear garbled.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserInfoDlg</name>
    <message>
        <source>to</source>
        <translation type="obsolete">to</translation>
    </message>
    <message>
        <source>IP:</source>
        <translation>IP:</translation>
    </message>
    <message>
        <source>Now</source>
        <translation>今</translation>
    </message>
    <message>
        <source>Age:</source>
        <translation>年:</translation>
    </message>
    <message>
        <source>Fax:</source>
        <translation>Fax:</translation>
    </message>
    <message>
        <source>Male</source>
        <translation>男性</translation>
    </message>
    <message>
        <source>UIN:</source>
        <translation type="obsolete">UIN:</translation>
    </message>
    <message>
        <source>Zip:</source>
        <translation>郵便番号:</translation>
    </message>
    <message>
        <source>done</source>
        <translation>完了</translation>
    </message>
    <message>
        <source>from</source>
        <translation type="obsolete">from</translation>
    </message>
    <message>
        <source> Day:</source>
        <translation>日:</translation>
    </message>
    <message>
        <source>&amp;Last</source>
        <translation>最後</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>メニュー</translation>
    </message>
    <message>
        <source>&amp;More</source>
        <translation>さらに</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>セーブ</translation>
    </message>
    <message>
        <source>&amp;Work</source>
        <translation>仕事</translation>
    </message>
    <message>
        <source>&amp;Filter: </source>
        <translation>フィルタ: </translation>
    </message>
    <message>
        <source>City:</source>
        <translation>都市:</translation>
    </message>
    <message>
        <source>P&amp;rev</source>
        <translation>前</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>名前:</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>次</translation>
    </message>
    <message>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>Last Online:</source>
        <translation>最後のオンライン:</translation>
    </message>
    <message>
        <source>Updating server...</source>
        <translation>サーバを更新中...</translation>
    </message>
    <message>
        <source> Year:</source>
        <translation>年:</translation>
    </message>
    <message>
        <source>Last Sent Event:</source>
        <translation>最後に送信したイベント</translation>
    </message>
    <message>
        <source>Last Checked Auto Response:</source>
        <translation>最後にチェックした自動応答</translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>情報</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to retrieve your settings.</source>
        <translation>あなたの設定をゲットするにはICQ
ネットワークに接続されている必要があります</translation>
    </message>
    <message>
        <source>Cellular:</source>
        <translation>携帯電話:</translation>
    </message>
    <message>
        <source>EMail 1:</source>
        <translation>メール 1:</translation>
    </message>
    <message>
        <source>EMail 2:</source>
        <translation>メール 2:</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>タイムアウト</translation>
    </message>
    <message>
        <source>Homepage:</source>
        <translation>ホームページ:</translation>
    </message>
    <message>
        <source>Timezone:</source>
        <translation>タイムゾーン</translation>
    </message>
    <message>
        <source>About:</source>
        <translation>情報</translation>
    </message>
    <message>
        <source>Alias:</source>
        <translation>ニックネーム</translation>
    </message>
    <message>
        <source>Old Email:</source>
        <translation>旧メール:</translation>
    </message>
    <message>
        <source>Female</source>
        <translation>女性</translation>
    </message>
    <message>
        <source> Month:</source>
        <translation>月:</translation>
    </message>
    <message>
        <source>Phone:</source>
        <translation>電話番号:</translation>
    </message>
    <message>
        <source>State:</source>
        <translation>州:</translation>
    </message>
    <message>
        <source>Last Received Event:</source>
        <translation>受信イベント:</translation>
    </message>
    <message>
        <source>Authorization Not Required</source>
        <translation>承認が必要でない</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 out of %4 matches</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;受信&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;送信&lt;/font&gt;] %4のうち%3がマッチ</translation>
    </message>
    <message>
        <source>GMT%1%1%1</source>
        <translation type="obsolete">GMT%1%1%1</translation>
    </message>
    <message>
        <source>[&lt;font color=&quot;%1&quot;&gt;Received&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;Sent&lt;/font&gt;] %3 to %4 of %5</source>
        <translation>[&lt;font color=&quot;%1&quot;&gt;受信&lt;/font&gt;] [&lt;font color=&quot;%2&quot;&gt;送信&lt;/font&gt;] %5のうち%3 から %4 まで</translation>
    </message>
    <message>
        <source>Country:</source>
        <translation>国:</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>失敗</translation>
    </message>
    <message>
        <source>server</source>
        <translation>サーバ</translation>
    </message>
    <message>
        <source>Authorization Required</source>
        <translation>承認が必要</translation>
    </message>
    <message>
        <source>INVALID USER</source>
        <translation>正しくないユーザ</translation>
    </message>
    <message>
        <source>Birthday:</source>
        <translation>誕生日:</translation>
    </message>
    <message>
        <source>Unknown (%1)</source>
        <translation>未知 (%1)</translation>
    </message>
    <message>
        <source>Department:</source>
        <translation>部署名:</translation>
    </message>
    <message>
        <source>Position:</source>
        <translation>役職:</translation>
    </message>
    <message>
        <source>Sorry, history is disabled for this person.</source>
        <translation>このユーザの履歴は見れません。</translation>
    </message>
    <message>
        <source>Licq - Info </source>
        <translation>Licq - 情報</translation>
    </message>
    <message>
        <source>Status:</source>
        <translation>ステータス</translation>
    </message>
    <message>
        <source>Address:</source>
        <translation>住所:</translation>
    </message>
    <message>
        <source>Updating...</source>
        <translation>更新中...</translation>
    </message>
    <message>
        <source>You need to be connected to the
ICQ Network to change your settings.</source>
        <translation>あなたの設定を変更するにはICQ
ネットワークに接続されている必要があります</translation>
    </message>
    <message>
        <source>Language 3:</source>
        <translation>言語3:</translation>
    </message>
    <message>
        <source>Language 1:</source>
        <translation>言語1:</translation>
    </message>
    <message>
        <source>Language 2:</source>
        <translation>言語2:</translation>
    </message>
    <message>
        <source>Unspecified</source>
        <translation>無指定</translation>
    </message>
    <message>
        <source>&amp;Update</source>
        <translation>更新</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <source>Retrieve</source>
        <translation>ゲット</translation>
    </message>
    <message>
        <source>Rever&amp;se</source>
        <translation>逆順</translation>
    </message>
    <message>
        <source>Gender:</source>
        <translation>性別:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>一般</translation>
    </message>
    <message>
        <source>&amp;History</source>
        <translation>過去ログ</translation>
    </message>
    <message>
        <source>Error loading history file: %1
Description: %2</source>
        <translation>履歴ファイル%1の読み込みに失敗
詳細: %2</translation>
    </message>
    <message>
        <source>Online Since:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Offline</source>
        <translation type="unfinished">オフライン</translation>
    </message>
    <message>
        <source>Keep Alias on Update</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Normally Licq overwrites the Alias when updating user details.
Check this if you want to keep your changes to the Alias.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Category:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Description:</source>
        <translation type="unfinished">説明</translation>
    </message>
    <message>
        <source>User has an ICQ Homepage </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User has no ICQ Homepage</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>M&amp;ore II</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(none)</source>
        <translation type="unfinished">(なし)</translation>
    </message>
    <message>
        <source>Occupation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Phone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">タイプ</translation>
    </message>
    <message>
        <source>Number/Gateway</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Country/Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Currently at:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>) </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>-</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>P&amp;icture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not Available</source>
        <translation type="unfinished">忙しい</translation>
    </message>
    <message>
        <source>Failed to Load</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;KDE Addressbook</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Email:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Retrieve</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>S&amp;end</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Clear</source>
        <translation type="unfinished">クリア</translation>
    </message>
    <message>
        <source>&amp;Browse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select your picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> is over %1 bytes.
Select another picture?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="unfinished">はい</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="unfinished">いいえ</translation>
    </message>
    <message>
        <source>Registration Date:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSelectDlg</name>
    <message>
        <source>&amp;User:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="unfinished">パスワード</translation>
    </message>
    <message>
        <source>&amp;Save Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="unfinished">キャンセル</translation>
    </message>
    <message>
        <source>Licq User Select</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendChatEvent</name>
    <message>
        <source>Clear</source>
        <translation>クリア</translation>
    </message>
    <message>
        <source>Multiparty: </source>
        <translation>複数人:</translation>
    </message>
    <message>
        <source>Invite</source>
        <translation>誘う</translation>
    </message>
    <message>
        <source>Chat with %2 refused:
%3</source>
        <translation>%2とのチャットは拒否されました:
%3</translation>
    </message>
    <message>
        <source> - Chat Request</source>
        <translation> - チャットリクエスト</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendCommon</name>
    <message>
        <source>No</source>
        <translation>いいえ</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>はい</translation>
    </message>
    <message>
        <source>done</source>
        <translation>終了</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>メニュー</translation>
    </message>
    <message>
        <source>&amp;Send</source>
        <translation>送信</translation>
    </message>
    <message>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>timed out</source>
        <translation>タイムアウト</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>メッセージ</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>ファイル転送</translation>
    </message>
    <message>
        <source>%1 refused %2, send through server.</source>
        <translation type="obsolete">%1は%2を拒否。サーバ経由で送信</translation>
    </message>
    <message>
        <source>Urgent</source>
        <translation>緊急</translation>
    </message>
    <message>
        <source>Send anyway</source>
        <translation>とりあえず送信</translation>
    </message>
    <message>
        <source>Drag Users Here
Right Click for Options</source>
        <translation>ユーザをここにドラッグ
右クリックでオプション</translation>
    </message>
    <message>
        <source>direct</source>
        <translation>直接</translation>
    </message>
    <message>
        <source>failed</source>
        <translation>失敗</translation>
    </message>
    <message>
        <source>via server</source>
        <translation>サーバ経由</translation>
    </message>
    <message>
        <source>U&amp;rgent</source>
        <translation>緊急</translation>
    </message>
    <message>
        <source>partial </source>
        <translation type="obsolete">部分的</translation>
    </message>
    <message>
        <source>%1 is in %2 mode:
%3
Send...</source>
        <translation>%1は%2でモード
%3
送信中...</translation>
    </message>
    <message>
        <source>Direct send failed,
send through server?</source>
        <translation>直接送信に失敗しました
サーバ経由で送りますか?</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Warning: Message will not be sent securely!</source>
        <translation type="obsolete">警告: メッセージは安全路で送信されません!</translation>
    </message>
    <message>
        <source>Sending </source>
        <translation>送信中</translation>
    </message>
    <message>
        <source>Chat Request</source>
        <translation>チャットリクエスト</translation>
    </message>
    <message>
        <source>Contact List</source>
        <translation>コンタクトリスト</translation>
    </message>
    <message>
        <source> to Contact List</source>
        <translation>コンタクトリストへ</translation>
    </message>
    <message>
        <source>M&amp;ultiple recipients</source>
        <translation>複数受信者</translation>
    </message>
    <message>
        <source>Se&amp;nd through server</source>
        <translation>サーバ経由で送信</translation>
    </message>
    <message>
        <source>SMS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning: Message can&apos;t be sent securely
through the server!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error! no owner set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>cancelled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 has joined the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 has left the conversation.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendContactEvent</name>
    <message>
        <source>Drag Users Here - Right Click for Options</source>
        <translation>ユーザをここにドラッグ - 右クリックでオプション</translation>
    </message>
    <message>
        <source> - Contact List</source>
        <translation> - コンタクトリスト</translation>
    </message>
</context>
<context>
    <name>UserSendFileEvent</name>
    <message>
        <source>You must specify a file to transfer!</source>
        <translation>転送するファイルを選択する必要があります</translation>
    </message>
    <message>
        <source>File(s): </source>
        <translation>ファイル:</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>ブラウズ</translation>
    </message>
    <message>
        <source> - File Transfer</source>
        <translation> - ファイル転送</translation>
    </message>
    <message>
        <source>File transfer with %2 refused:
%3</source>
        <translation>%2とのファイル転送は拒否されました:
%3</translation>
    </message>
    <message>
        <source>Select files to send</source>
        <translation>送信ファイルを選択</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">編集</translation>
    </message>
    <message>
        <source>No reason provided</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendMsgEvent</name>
    <message>
        <source>&amp;No</source>
        <translation>いいえ</translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation>はい</translation>
    </message>
    <message>
        <source> - Message</source>
        <translation>- メッセージ</translation>
    </message>
    <message>
        <source>You didn&apos;t edit the message.
Do you really want to send it?</source>
        <translation>メッセージを編集していません。
本当に送りますか?</translation>
    </message>
</context>
<context>
    <name>UserSendSmsEvent</name>
    <message>
        <source>Phone : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Chars left : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> - SMS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You didn&apos;t edit the SMS.
Do you really want to send it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Yes</source>
        <translation type="unfinished">はい</translation>
    </message>
    <message>
        <source>&amp;No</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserSendUrlEvent</name>
    <message>
        <source> - URL</source>
        <translation> - URL</translation>
    </message>
    <message>
        <source>URL : </source>
        <translation>URL : </translation>
    </message>
    <message>
        <source>No URL specified</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>UserViewEvent</name>
    <message>
        <source>Chat</source>
        <translation>チャット</translation>
    </message>
    <message>
        <source>&amp;Join</source>
        <translation>参加</translation>
    </message>
    <message>
        <source>&amp;Menu</source>
        <translation>メニュー</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>見る</translation>
    </message>
    <message>
        <source>Nex&amp;t</source>
        <translation>次</translation>
    </message>
    <message>
        <source>A&amp;dd %1 Users</source>
        <translation>%1ユーザを追加</translation>
    </message>
    <message>
        <source>Aut&amp;o Close</source>
        <translation>自動で閉じる</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>&amp;Quote</source>
        <translation>引用</translation>
    </message>
    <message>
        <source>&amp;Reply</source>
        <translation>返信</translation>
    </message>
    <message>
        <source>A&amp;ccept</source>
        <translation>受信</translation>
    </message>
    <message>
        <source>Normal Click - Close Window
&lt;CTRL&gt;+Click - also delete User</source>
        <translation>通常クリック - ウィンドウ閉じる
&lt;CTRL&gt;+クリック - さらにユーザを除去</translation>
    </message>
    <message>
        <source>File Transfer</source>
        <translation>ファイル転送</translation>
    </message>
    <message>
        <source>Start Chat</source>
        <translation>チャット開始</translation>
    </message>
    <message>
        <source>
--------------------
Request was cancelled.</source>
        <translation>
--------------------
リクエストはキャンセルされました</translation>
    </message>
    <message>
        <source>View URL failed</source>
        <translation type="obsolete">URLを閲覧できません</translation>
    </message>
    <message>
        <source>A&amp;dd User</source>
        <translation>ユーザを追加</translation>
    </message>
    <message>
        <source>A&amp;uthorize</source>
        <translation>承認</translation>
    </message>
    <message>
        <source>&amp;Refuse</source>
        <translation>拒否</translation>
    </message>
    <message>
        <source>Nex&amp;t (%1)</source>
        <translation>次 (%1)</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>転送</translation>
    </message>
    <message>
        <source>&amp;View Email</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq is unable to find a browser application due to an internal error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Licq is unable to start your browser and open the URL.
You will need to start the browser and open the URL manually.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;View Info</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>VerifyDlg</name>
    <message>
        <source>Retype the letters shown above:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>Licq - New Account Verification</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>WharfIcon</name>
    <message>
        <source>Left click - Show main window
Middle click - Show next message
Right click - System menu</source>
        <translation>左クリック - メインウィンドウを表示
真中クリック - 次メッセージを表示
右クリック - システムメニュー</translation>
    </message>
</context>
</TS>
