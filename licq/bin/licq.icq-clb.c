/* 
 * icq .clb to licq contact list converter
 * written by Philip Nelson (phatfil@optushome.com.au)
 * uin: 16385042
 * 
 * Minor fix by Jon Keating
 */

#include <stdio.h>
#define MAXLINE 128

typedef struct icqInfo
{
	char uin[15];
	char name[20];
	char group[20];
	struct icqInfo *link;
} icqInfo;

icqInfo * Add(icqInfo *, char *, char *, char *);

typedef struct groups
{
	char grpname[20];
	int groupid;
	struct groups *link;
} groups;

void printInfo(icqInfo *, groups *);
groups * addGroup(groups *, char *);
void printGroups(groups *);
int foundGroup(groups *, char *);
groups * doGroups(icqInfo *, groups *);
int getGroupId(char *, groups *);
char basedir[1000];
char clb[100];

int main(int numargs, char *argv[]) {
	int i;
	int j;
	FILE *f;
	char line[MAXLINE];
	char *name;
	char *group;
	char *uin;
	char accept;
	icqInfo *details_pointer = NULL;
	groups *groups_pointer = NULL;

	if(numargs != 3) {
		printf("Usage: %s <icqclbfile> <licqbasedir>\n", argv[0]);
		exit(1);
	}

	printf("**WARNING!!** It is HIGHLY recommended you backup your\nlicq base directory (default ~/.licq) before using this\ntool. If you haven't done so, please do, then rerun me.\nContinue? (y/n) ");
	accept = getchar();
	accept = toupper(accept);
	if(accept != 'Y') {
		printf("Exiting...\n");
		exit(0);
	}
	
	strncpy(clb, argv[1], 100);
	clb[99] = '\0';
	strncpy(basedir, argv[2], 1000);
	basedir[999] = '\0';

	f = fopen(clb, "r");
	if(f) {
		int k;
		for(k=1;fgets(line, MAXLINE, f);k++) {
			j=0;

			for(i=0;line[i] != '\r';i++) {
				if(line[i] == ';') {
					*(line+i) = '\0';
					if(j==0) {
						group = line;
						uin = line+i+1;
					}else if(j==1) {
						name = line+i+1;
					}else if(j==2) {
					} else {
						printf("Error parsing file! line: %d (make sure name doesn't contains any semilcolons)\n", k);
						exit(1);
					}
					j++;
				}
			}
  			
			details_pointer = Add(details_pointer, name, group, uin);
		}
	}else{
		printf("Couldn't open %s\n", argv[1]);
	}
	
	groups_pointer = doGroups(details_pointer, groups_pointer);
	printInfo(details_pointer, groups_pointer);
	printGroups(groups_pointer);
	printf("Done! Please icq 16385042 and tell him how useful this was :-)\n");
}

icqInfo * Add(icqInfo * details_pointer, char *name, char *group, char *uin)
{
	icqInfo * lp = details_pointer;
        if(details_pointer) {
		while(details_pointer->link)
			details_pointer=details_pointer->link;
       
		details_pointer->link = (struct icqInfo *)malloc(sizeof(icqInfo));
		details_pointer = details_pointer->link;
		details_pointer->link = NULL;
		strncpy(details_pointer->name, name, 20);
		strncpy(details_pointer->group, group, 20);
		strncpy(details_pointer->uin, uin, 15);
		details_pointer->name[19] = '\0';
		details_pointer->group[19] = '\0';
		details_pointer->uin[14] = '\0';
		return lp;
	} else {
		details_pointer = (struct icqInfo *)malloc(sizeof(icqInfo));
		details_pointer->link = NULL;
		strncpy(details_pointer->name, name, 20);
		strncpy(details_pointer->group, group, 20);
		strncpy(details_pointer->uin, uin, 20);
		details_pointer->name[19] = '\0';
		details_pointer->group[19] = '\0';
		details_pointer->uin[14] = '\0';
		return details_pointer;
	}
}

void printInfo(icqInfo *details_pointer, groups *groups_pointer)
{
	FILE *users;
	FILE *user;
	int i;
	int groupid;
	char usersfile[1011];
	strcpy(usersfile, basedir);
	strcat(usersfile, "/users.conf");
	users=fopen(usersfile, "w");
	if(users==NULL) {
		printf("Error: couldn't open %s for writing\n", usersfile);
		exit(1);
	}
	fputs("[users]\n", users);
	for(i=1;details_pointer;i++) {
		char uinfile[1000];
		fprintf(users, "User%d = %s\n", i, details_pointer->uin);
		sprintf(uinfile, "%s/users/%s.uin", basedir, details_pointer->uin);
		user=fopen(uinfile, "w");
		fputs("[user]\n", user);
		fprintf(user, "Alias = %s\n", details_pointer->name);
		groupid = getGroupId(details_pointer->group, groups_pointer);
		if(groupid) {
			fprintf(user, "Groups.User = %d\n", groupid);
		}
		details_pointer = details_pointer->link;
	}
	fprintf(users, "NumOfUsers = %d", i-1);
	fclose(users);
	fclose(user);
}

int getGroupId(char *group, groups *groups_pointer)
{
	if(groups_pointer == NULL) {
		printf("weird error!\n");
	} else {
		while(groups_pointer) {
			if(!strcmp(group, groups_pointer->grpname)) {
				return groups_pointer->groupid;
			}
			groups_pointer = groups_pointer->link;
		}
	}
	return 0;
}

groups *doGroups(icqInfo *details_pointer, groups *groups_pointer)
{
	if(details_pointer == NULL) {
		printf("weird error!\n");
	} else {
		while(details_pointer) {
			if(!foundGroup(groups_pointer, details_pointer->group)) {
				groups_pointer = addGroup(groups_pointer, details_pointer->group);
			}
			details_pointer=details_pointer->link;
		}
	}
	return groups_pointer;
}

int foundGroup(groups *groups_pointer, char *groupname)
{
	int found = 1;
	if(groups_pointer == NULL) {
		found = 1;
	} else {
		while(groups_pointer) {
			if(!strcmp(groups_pointer->grpname, groupname)) {
				found = 0;
			}
			groups_pointer = groups_pointer->link;
		}
	}
	if(found) {
		return 0;
	}else{
		return 1;
	}
}
		   
void printGroups(groups *groups_pointer)
{
	int i;

	FILE *conf;
	FILE *tmp;
	char line[100];
	char brokenline[100];
	char *borked=NULL;
	char conffile[1000];
	char tmpfile[1000];
	char mvcmd[1000];
	strcpy(conffile, basedir);
	strcat(conffile, "/licq.conf");
	strcpy(tmpfile, basedir);
	strcat(tmpfile, "/licq.conf.tmp");
	conf=fopen(conffile, "r");
	if(conf==NULL) {
		printf("Error: couldn't open %s for reading!\n", conffile);
		exit(1);
	}
	tmp=fopen(tmpfile, "w");
	if(tmp==NULL) {
		printf("Error: couldn't write to temporary file %s (check dir permissions)\n", tmpfile);
		exit(1);
	}

	while(fgets(line, 100, conf)) {
		if(!strncmp(line, "[groups]", 8)) {
			while(fgets(line, 100, conf)) {
				if(!strncmp(line, "[", 1)) {
					break;
				}
			}
		}
		fprintf(tmp, "%s", line);
	}
	fclose(conf);
	fputs("[groups]\n", tmp);
	for(i=0;groups_pointer;i++) {
		fprintf(tmp, "Group%d.name = %s\n", i+1, groups_pointer->grpname);
		groups_pointer = groups_pointer->link;
	}
	fprintf(tmp, "NumOfGroups = %d\n", i);
	fputs("DefaultGroup = 0\n",tmp);
	fputs("NewUserGroup = 1\n",tmp);
	fclose(tmp);
	rename(tmpfile, conffile);
}

groups *addGroup(groups *groups_pointer, char *name)
{
	int i=1;
	groups *lp = groups_pointer;
	if(groups_pointer) {
		i=2;
		while(groups_pointer->link) {
			groups_pointer = groups_pointer->link;
			i++;
		}   
		groups_pointer->link = (struct groups *)malloc(sizeof(groups));
		groups_pointer = groups_pointer->link;
		groups_pointer->link = NULL;
		strcpy(groups_pointer->grpname, name);
		groups_pointer->groupid = power(2, i-1);
		return lp;
	} else {
		groups_pointer = (struct groups *)malloc(sizeof(groups));
		groups_pointer->link = NULL;
		strcpy(groups_pointer->grpname, name);
		groups_pointer->groupid = power(2, i-1);
		i++;
		return groups_pointer;
	}
}

int power(int base, int n) {
    int     i,
            p;
    p = 1;
    for (i = 1; i <= n; ++i)
	p *= base;
    return p;
}








