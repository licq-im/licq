#ifndef AUTORESPONSE_CONF_H
#define AUTORESPONSE_CONF_H

static const char* default_autoResponseText[] = {

    // Away messages
    "I am currently away from ICQ.\nPlease leave your message and I will get back to you as soon as I return!",
    "I'm out to lunch.\nI will return shortly.",
    "Don't go anywhere!\nI'll be back in a jiffy.",
    "I'm out with the dog.\nBe back when he's finished",
    "Went out for a smoke.",
    "On my coffee break.",
    "Went to get some fresh air.",
    "Enter your message here",

    // N/A messages

    "I am out'a here.\nSee you tomorrow!",
    "Give it up! I 'm not in!",
    "I'm closed for the weekend/holidays.",
    "Gone fishin'!",
    "I'm sleeping. Don't wake me.",
    "Went home.\nHad to feed the kids.",
    "Gone for good.",
    "Enter your message here",

    // Occupied messages

    "Please do not disturb me now.\nDisturb me later.\nOnly urgent messages, please!",
    "I'm currently in a meeting. I can't be disturbed.\nOnly urgent messages, please!",
    "Don't disturb my concentration!\nOnly urgent messages, please!",
    "I'm on the phone with a very important client. Don't disturb me!\nOnly urgent messages please!",
    "I can't chat with you now. I'm busy.\nOnly urgent messages, please!",
    "Can't you see I'm working?\nOnly urgent messages, please!",
    "I am conversing with my colleagues.\nOnly urgent messages, please!",
    "User is occupied.\nOnly urgent messages will be delivered.",

    // Do not Disturb messages 

    "Please do not disturb me now. Disturb me later.",
    "I'm currently in a meeting.\nI can't be disturbed.",
    "Don't disturb my concentration!",
    "I'm on the phone with a very important client.\nDon't disturb me!",
    "I can't chat with you now. I'm busy.",
    "Can't you see I'm working?",
    "Fiddling around some things and I'm quite busy with that.\nSo please disturb me later!",
    "Enter your message here",

    // Free For Chat messages 

    "We'd love to hear what you have to say. Join our chat.",
    "Come in and join my chat room!",
    "Don't miss the fun! Join our chat!",
    "What are you waiting for? Come in!",
    "We'd love to hear\nwhat you have to say.\nJoin our chat!",
    "Enter your chat room message here",
    "Enter your chat room message here",
    "Enter your chat room message here",
};

static const char* default_autoResponseHeader[] = {

    // Away messages
    
    "Away",
    "Lunch",
    "Be right back",
    "Dog Walk",
    "Smoke",
    "Coffee",
    "Air",
    "--Empty--",

    
    // N/A messages
    
    "Out for the day",
    "Not here",
    "Closed",
    "Fishing",
    "Sleeping",
    "Kids",
    "Gone",
    "--Empty--",

    
    // Occupied messages
    
    "Busy",
    "Meeting",
    "Concentration",
    "On the Phone",
    "Can't chat",
    "Working",
    "Conversing",
    "--Empty--",

    
    // Do Not Disturb messages
    
    "Do Not Disturb",
    "Meeting",
    "Concentration",
    "On the Phone",
    "Can't chat",
    "Working",
    "Fiddling",
    "--Empty--",

    
    // Free For Chat messages
    
    "Chat",
    "Come in",
    "Fun",
    "Don't Wait",
    "Hear",
    "--Empty--",
    "--Empty--",
    "--Empty--"
};


#endif
