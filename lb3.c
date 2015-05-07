#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<termios.h>
#define BOOKFILE "books.txt"
#define MEMBERFILE "members.txt"
#define MAX_BORROW 3
#define MAX_DUP_BOOKS 5    //max books with same name
#define PASSWORD "root"

typedef struct Books{
    int id[MAX_DUP_BOOKS];
    char *name;
    char *author;
    int status[MAX_DUP_BOOKS];                 //1 for available, 0 for not available
    struct Books *left, *right, *parent;
} Book;

typedef struct Members{
    int id;
    char *name;
    int borrowedBookId[MAX_BORROW];
    struct Members *left, *right, *parent;
} Member;

/*関数プロトタイプ*/
void IssueABook();
void ReturnBook();
void DisplayAllBooks();
void DisplayAllMembers();
void AddNewBook();
void AddNewMember();
void FindMember();
void FindBook();
void DeleteMember();
void DeleteBook();
void InsertBookInTree(Book **l, Book *b, Book *parent);
void InsertMemberInTree(Member **l, Member *b, Member *parent);
void LoadBooks(Book **bookRoot);
void LoadMembers(Member **memRoot);
void PrintBook(Book *b);
void PrintBooks(Book *b);
void PrintMember(Member *b);
void PrintMembers(Member *m);
void SaveBooks(Book *bookRoot, FILE **f);
void SaveMembers(Member *memRoot, FILE **f);
void WriteBookToFile(FILE **f, Book *b);
void WriteMemberToFile(FILE **f, Member *m);
Book *SearchBookByName(Book *bookRoot, char name[]);
Book *SearchBookById(Book *bookRoot, int id);
Member *SearchMemberById(Member *memRoot, int id);
void DeleteBookByNode(Book **bookRoot, Book *b);
void DeleteMemberByNode(Member **memRoot, Member *m);
void AdminLogin(int *admin);

Book *bookRoot = NULL;
Member *memRoot = NULL;
int maxBookId = 0, maxMemId = 0;

int main(){
    char inputs[10];
    int input;
    int admin = 0;  //0 for NO, 1 for YES
    
    LoadBooks(&bookRoot);
    LoadMembers(&memRoot);
    
    while(1){
        //Main Menu
        printf("\n\n\t\t\t\t UEC Library\n\n");
        printf("1. Issue a book\n");
        printf("2. Return book\n");
        printf("3. Display all books\n");
        printf("4. Find a book\n");
        if(admin == 0){
            printf("5. Admin Login\n");
            printf("6. Exit\n");
        }else{
            printf("5. Add new book\n");
            printf("6. Delete book\n");
            printf("7. Add new member\n");
            printf("8. Display all members\n");
            printf("9. Find member\n");
            printf("10. Delete member\n");
            printf("11. Logout\n");
            printf("12. Exit\n");
        }
        
        printf("\nEnter your choice: ");
        fgets(inputs, 10, stdin);
        input = atoi(inputs);
        
        if(input == 0){
            printf("Saving database...\n");
            FILE *f = fopen(BOOKFILE, "w");
            SaveBooks(bookRoot, &f);
            fclose(f);
            f = fopen(MEMBERFILE, "w");
            SaveMembers(memRoot, &f);
            fclose(f);
            exit(1);
        }
        
        if(input <= 0)
        exit(1);
        
        if(admin == 0){
            switch(input){
                case 1:
                IssueABook();
                break;
                case 2:
                ReturnBook();
                break;
                case 3:
                DisplayAllBooks();
                break;
                case 4:
                FindBook();
                break;
                case 5:
                AdminLogin(&admin);
                break;
                default:
                printf("Saving database...\n");
                FILE *f = fopen(BOOKFILE, "w");
                SaveBooks(bookRoot, &f);
                fclose(f);
                f = fopen(MEMBERFILE, "w");
                SaveMembers(memRoot, &f);
                fclose(f);
                exit(1);
            }
        }else{
            switch(input){
                case 1:
                IssueABook();
                break;
                case 2:
                ReturnBook();
                break;
                case 3:
                DisplayAllBooks();
                break;
                case 4:
                FindBook();
                break;
                case 5:
                AddNewBook();
                break;
                case 6:
                DeleteBook();
                break;
                case 7:
                AddNewMember();
                break;
                case 8:
                DisplayAllMembers();
                break;
                case 9:
                FindMember();
                break;
                case 10:
                DeleteMember();
                break;
                case 11:
                admin = 0;
                break;
                case 12:
                default:
                printf("Saving database...\n");
                FILE *f = fopen(BOOKFILE, "w");
                SaveBooks(bookRoot, &f);
                fclose(f);
                f = fopen(MEMBERFILE, "w");
                SaveMembers(memRoot, &f);
                fclose(f);
                exit(1);
            }
        }
    }
    
    return 0;
}

void AdminLogin(int *admin){
    while(1){
        static struct termios oldt, newt;
        int i = 0;
        int c, SIZE = 30;
        char password[SIZE];
        
        printf("Enter password: ");
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ECHO);
        tcsetattr( STDIN_FILENO, TCSANOW, &newt);
        
        while ((c = getchar())!= '\n' && c != EOF && i < SIZE){
            password[i++] = c;
        }
        password[i] = '\0';
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        
        if(strcmp(password, PASSWORD) == 0){
            printf("\nLogin successful! Welcome Admin!\n\n");
            *admin = 1;
            sleep(1);
            break;
        }else{
            printf("\nIncorrect password, enter again. Enter '-1' to return.\n");
        }
    }
}

void IssueABook(){
    int m_id, b_id, i;
    int borrowed=0;
    Member *m;
    Book *b;
    char buf[100];
    
    printf("\n\n\n");
    printf("\t\t\t\tBOOK ISSUE\n\n");
    
    while(1){
        printf("Enter '-1' to return to main menu.\n");
        printf("Enter Member ID: ");
        
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("Error occurred, try again.\n");
            continue;
        }
        
        if(strcmp(buf, "-1\n") == 0){
            return;
        }
        
        if((m_id = atoi(buf)) == 0){
            printf("Error occurred, try again.\n");
            continue;
        }
        
        if((m = SearchMemberById(memRoot, m_id)) == NULL){
            printf("No such member exists, try again.\n");
            continue;
        }
        
        printf("\n\nMember Name: %s", m->name);
        printf("\nBorrow Status: ");
        for(i=0; i<MAX_BORROW; i++){
            if(m->borrowedBookId[i] == -1)
            printf("%c  ", 'X');
            else{
                borrowed++;
                printf("%d  ", m->borrowedBookId[i]);
            }
        }
        
        if(borrowed == MAX_BORROW){
            printf("\n\nSorry, you can issue only %d books at a time, return other books first.\n\n", MAX_BORROW);
            sleep(2);
            return;
        }
        
        printf("\n");
        break;
    }
    
    while(1){
        int bookAvailable = 0, bookCount = 0;
        
        printf("\nEnter '-1' to return to main menu.\n");
        printf("Enter '-2' to search Book by ID instead.\n");
        printf("Enter Book Name: ");
        
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("Error occurred, try again.\n");
            continue;
        }
        if(strcmp(buf, "-1\n") == 0){
            return;
        }else if(strcmp(buf, "-2\n") == 0){
            printf("Enter Book ID: ");
            if(fgets(buf, sizeof(buf), stdin) == NULL){
                printf("Error occurred, try again.\n");
                continue;
            }
            
            if((b_id = atoi(buf)) == 0){
                printf("Error occurred, try again.\n");
                continue;
            }
            
            b = SearchBookById(bookRoot, b_id);
        }else{
            buf[strlen(buf)-1] = '\0';
            b = SearchBookByName(bookRoot, buf);
        }
        
        if(b == NULL){
            printf("\nNo such book found, try again.\n");
            continue;
        }
        
        for(i=0; i<MAX_DUP_BOOKS; i++){
            if(b->id[i] != -1){
                bookCount++;
            }
            if(b->status[i] == 1)
            bookAvailable = 1;
        }
        
        printf("\n\t\t\t\t\tBook(s) found");
        printf("\n\n%30s %25s %10s %15s\n", "Book", "Author", "Book ID", "Availability");
        printf("---------------------------------------------------------------------------------------------\n");
        PrintBook(b);
        printf("---------------------------------------------------------------------------------------------\n");
        
        if(bookAvailable == 0){
            printf("Sorry, this book is currently not available. Try again later.\n\n");
            continue;
        }
        
        if(bookCount>1){
            while(1){
                printf("\nMultiple entries found, please specify the Book ID: ");
                fgets(buf, 5, stdin);
                int found = 0, availCheck = 1;
                
                if((b_id = atoi(buf)) == 0){
                    printf("Error occurred, try again.\n");
                    continue;
                }
                
                for(i=0; i<MAX_DUP_BOOKS; i++){
                    if(b->id[i] == b_id){
                        if(b->status[i] == -1){
                            printf("This book is currently not available, try again later.\n");
                            sleep(1);
                            availCheck = 0;
                        }
                        found = 1;
                        b->status[i] = -1;
                        break;
                    }
                }
                
                if(availCheck == 0){
                    continue;
                }
                
                if(found == 0){
                    printf("Incorrect Book ID, try again.\n");
                    continue;
                }else{
                    for(i=0; i<MAX_BORROW; i++){
                        if(m->borrowedBookId[i] == -1){
                            m->borrowedBookId[i] = b_id;
                            break;
                        }
                    }
                }
                
                printf("Book successfully issued.\n");
                sleep(1);
                return;
            }
        }else{
            printf("\nAre you sure you want to borrow this Book?(y/n) ");
            if(fgets(buf, 5, stdin) == NULL)
            return;
            buf[strlen(buf)-1] = '\0';
            
            if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
                
                for(i=0; i<MAX_DUP_BOOKS; i++){
                    if(b->status[i] != -1){
                        b_id = b->id[i];
                        b->status[i] = -1;
                        break;
                    }
                }
                
                for(i=0; i<MAX_BORROW; i++){
                    if(m->borrowedBookId[i] == -1){
                        m->borrowedBookId[i] = b_id;
                        break;
                    }
                }
                
                printf("\nBook successfully issued.\n\n");
                sleep(1);
                return;
            }
        }
    }
}

void ReturnBook(){
    int m_id, b_id, i;
    char buf[100];
    Member *m;
    Book *b;
    
    printf("\n\n\t\t\t\tReturn Book\n\n");
    
    while(1){
        int borrowed = 0;
        
        printf("Enter '-1' to return to main menu.\n");
        printf("Enter Member ID: ");
        
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("Error occurred, try again.\n");
            continue;
        }
        
        if(strcmp(buf, "-1\n") == 0){
            return;
        }
        
        if((m_id = atoi(buf)) == 0){
            printf("Error occurred, try again.\n");
            continue;
        }
        
        if((m = SearchMemberById(memRoot, m_id)) == NULL){
            printf("No such member exists, try again.\n");
            continue;
        }
        
        printf("\n\nMember Name: %s", m->name);
        printf("\nBorrow Status: ");
        for(i=0; i<MAX_BORROW; i++){
            if(m->borrowedBookId[i] == -1)
            printf("%c  ", 'X');
            else{
                borrowed++;
                printf("%d  ", m->borrowedBookId[i]);
            }
        }
        
        if(borrowed == 0){
            printf("\n\nYou haven't borrowed any books, returning to main menu.\n\n");
            sleep(2);
            return;
        }
        
        printf("\n\n");
        break;
    }
    
    while(1){
        int borrowCheck = 0;
        
        printf("\nEnter '-1' to return to main menu.\n");
        printf("Enter Book ID: ");
        
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("Error occurred, try again.\n");
            continue;
        }
        if((b_id = atoi(buf)) == -1){
            return;
        }else if(b_id == 0){
            printf("Error occurred, try again.\n");
            continue;
        }
        
        for(i=0; i<MAX_BORROW; i++){
            if(m->borrowedBookId[i] == b_id)
            borrowCheck = 1;
        }
        
        if(borrowCheck == 0){
            printf("You haven't borrowed this book, try again.\n");
            continue;
        }
        
        b = SearchBookById(bookRoot, b_id);
        
        if(b == NULL){
            printf("\nNo such book found, try again.\n");
            continue;
        }
        
        printf("Book Name: %s\n", b->name);
        
        printf("\nAre you sure you want to return this Book?(y/n) ");
        if(fgets(buf, 5, stdin) == NULL)
        return;
        buf[strlen(buf)-1] = '\0';
        
        if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
            
            for(i=0; i<MAX_DUP_BOOKS; i++){
                if(b->id[i] == b_id){
                    b->status[i] = 1;
                    break;
                }
            }
            
            for(i=0; i<MAX_BORROW; i++){
                if(m->borrowedBookId[i] == b_id){
                    m->borrowedBookId[i] = -1;
                    break;
                }
            }
            
            printf("\nBook successfully returned.\n\n");
            sleep(1);
            return;
        }
    }
}

void DisplayAllBooks(){
    char buf[10];
    
    printf("\n\n\t\t\t\t\tUEC Library Book List");
    printf("\n\n%30s %25s %10s %15s\n", "Book", "Author", "Book ID", "Availability");
    printf("---------------------------------------------------------------------------------------------\n");
    
    PrintBooks(bookRoot);
    
    printf("---------------------------------------------------------------------------------------------\n");
    
    printf("\nPress any key to return ");
    fgets(buf, 10, stdin);
}

void FindBook(){
    while(1){
        char buf[100];
        Book *b;
        int i, bookCount=0, choice;
        
        printf("\n\n\t\t\t\t Find Book");
        printf("\n\nEnter '-1' for Main menu\n");
            printf("Enter '-2' to Find by ID instead");
        printf("\nEnter Book name: ");
        
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("Error occurred, try again.");
            continue;
        }
        
        if(strcmp(buf, "-1\n") == 0)
        return;
        
        if(strcmp(buf, "-2\n") == 0){
            printf("\nEnter Book ID: ");
            if(fgets(buf, 5, stdin) == NULL){
                printf("Error occurred, try again.");
                continue;
            }
            
            if((choice = atoi(buf)) == 0){
                printf("Error occurred, try again.\n");
                continue;
            }
            
            b = SearchBookById(bookRoot, choice);
        }else{
            buf[strlen(buf)-1] = '\0';
            b = SearchBookByName(bookRoot, buf);
        }
        
        if(b == NULL){
            printf("\nBook not found.\n");
            sleep(1);
            continue;
        }
        
        for(i=0; i<MAX_DUP_BOOKS; i++){
            if(b->id[i] != -1){
                bookCount++;
            }
        }
        
        printf("\n\t\t\t\t\tBook(s) found");
        printf("\n\n%30s %25s %10s %15s\n", "Book", "Author", "Book ID", "Availability");
        printf("---------------------------------------------------------------------------------------------\n");
        PrintBook(b);
        printf("---------------------------------------------------------------------------------------------\n");
    }
}

void FindMember(){
    int id;
    char buf[20];
    Member *m;
    
    printf("\n\n\n\t\t\t Find Member");
    
    while(1){
        printf("\n\n(Enter '-1' to return to main menu.)");
        printf("\nEnter member ID: ");
        if(fgets(buf, 5, stdin) != NULL){
            if(strcmp(buf, "-1\n") == 0)
            return;
            if((id = atoi(buf)) == 0){
                printf("Error occurred\n");
                sleep(1);
                return;
            }
            
            m = SearchMemberById(memRoot, id);
            
            if(m == NULL){
                printf("Member not found. Try again.\n");
                continue;
            }
            
            printf("\n\n\t\t\t Member found");
            printf("\n\n%25s %5s %22s\n", "Name", "ID", "Borrowed Book IDs");
            printf("-----------------------------------------------------------\n");
            PrintMember(m);
            printf("-----------------------------------------------------------\n");
        }
    }
}

void DisplayAllMembers(){
    printf("\n\n\t\t\tUEC Library Member List");
    printf("\n\n%25s %5s %22s\n", "Name", "ID", "Borrowed Book IDs");
    printf("-----------------------------------------------------------\n");
    PrintMembers(memRoot);
    printf("-----------------------------------------------------------\n");
}

void AddNewBook(){
    while(1){
        Book b, *p;
        char buf[100];
        int id, i, canInsert = 0;  //1 for okay, 0 for not okay
        
        id = maxBookId + 1;
        
        printf("Enter '-1' to return to main menu.\n");
        printf("\nSystem generated Book ID: %d\n", id);
        printf("Book Name: ");
        
        if(fgets(buf, 100, stdin) == NULL){
            printf("NULL returned from fgets, exiting...\n");
            exit(1);
        }
        
        if(strcmp(buf, "-1\n") == 0)
        return;
        
        buf[strlen(buf)-1] = '\0';
        b.name = (char*)malloc(strlen(buf)+1);
        strcpy(b.name, buf);
        
        printf("Author: ");
        if(fgets(buf, 100, stdin) == NULL){
            printf("NULL returned from fgets, exiting...\n");
            exit(1);
        }
        
        if(strcmp(buf, "-1\n") == 0)
        return;
        
        buf[strlen(buf)-1] = '\0';
        b.author = (char*)malloc(strlen(buf)+1);
        strcpy(b.author, buf);
        
        p = SearchBookByName(bookRoot, b.name);
        
        if(p == NULL){   //Book with same name doesn't exist
            b.id[0] = id;
            b.status[0] = 1;
            canInsert = 1;
            for(i=1; i<MAX_DUP_BOOKS; i++){
                b.id[i] = -1;
                b.status[i] = -1;
            }
            InsertBookInTree(&bookRoot, &b, NULL);
        }else{
            for(i=0; i<MAX_DUP_BOOKS; i++){
                if(p->id[i] == -1){
                    canInsert = 1;
                    p->id[i] = id;
                    p->status[i] = 1;
                    break;
                }
            }
        }
        
        if(canInsert == 0){
            printf("\nNo more books with this name can be inserted.\n\n");
            sleep(1);
            continue;
        }
        
        maxBookId++;
        
        printf("\nDo you want to add another book?(y/n): ");
        if(fgets(buf, 5, stdin) == NULL)
        return;
        buf[strlen(buf)-1] = '\0';
        if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
            continue;
        }else
            break;
    }
}

void AddNewMember(){
    Member b, *p;
    char buf[100];
    int i, canInsert = 0;  //1 for okay, 0 for not okay
    
    while(1){
        b.id = ++maxMemId;
        printf("\nSystem generated Member ID: %d\n", b.id);
        
        printf("Member Name: ");
        if(fgets(buf, 100, stdin) == NULL){
            printf("NULL returned from fgets, exiting...\n");
            exit(1);
        }
        buf[strlen(buf)-1] = '\0';
        b.name = (char*)malloc(strlen(buf)+1);
        strcpy(b.name, buf);
        
        for(i=0; i<MAX_BORROW; i++){
            b.borrowedBookId[i] = -1;
        }
        InsertMemberInTree(&memRoot, &b, NULL);
        
        printf("\n\nMember added.\n");
        printf("\nDo you want to add another member?(y/n): ");
        if(fgets(buf, 5, stdin) == NULL)
        return;
        buf[strlen(buf)-1] = '\0';
        if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
            continue;
        }else
            break;
    }
}

void DeleteMember(int m_id){
    int id;
    char buf[20];
    Member *m;
    
    printf("\n\n\n\t\t\t Delete Member");
    
    while(1){
        printf("\n\n(Enter '-1' to return to main menu.)");
        printf("\nEnter member ID: ");
        if(fgets(buf, 5, stdin) != NULL){
            if(strcmp(buf, "-1\n") == 0)
            return;
            if((id = atoi(buf)) == 0){
                printf("Error occurred\n");
                sleep(1);
                return;
            }
            
            m = SearchMemberById(memRoot, id);
            
            if(m == NULL){
                printf("Member not found. Try again.\n");
                continue;
            }
            
            printf("\n\n\t\t\t Member found");
            printf("\n\n%25s %5s %22s\n", "Name", "ID", "Borrowed Book IDs");
            printf("-----------------------------------------------------------\n");
            PrintMember(m);
            printf("-----------------------------------------------------------\n");
            
            printf("\nAre you sure you want to delete this member? (y/n): ");
            if(fgets(buf, 5, stdin) == NULL)
            return;
            buf[strlen(buf)-1] = '\0';
            if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
                DeleteMemberByNode(&memRoot, m);
                printf("\nMember successfully deleted\n\n");
                sleep(1);
                return;
            }
        }
    }
}

void DeleteBook(){
    while(1){
        char buf[100];
        Book *b;
        int i, bookCount=0, choice, deleted = 0;
        
        printf("\n\n\t\t\t\t Delete Book");
        printf("\n\nEnter '-1' for Main menu\n");
            printf("Enter '-2' to Delete by ID instead");
        
        printf("\nEnter Book name: ");
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("Error occurred, try again.");
            continue;
        }
        
        if(strcmp(buf, "-1\n") == 0)
        return;
        
        if(strcmp("buf", "-2\n") == 0){
            printf("\nEnter Book ID: ");
            if(fgets(buf, 5, stdin) == NULL){
                printf("Error occurred, try again.");
                continue;
            }
            
            if((choice = atoi(buf)) == 0){
                printf("Error occurred, try again.\n");
                continue;
            }
            
            b = SearchBookById(bookRoot, choice);
        }else{
            buf[strlen(buf)-1] = '\0';
            b = SearchBookByName(bookRoot, buf);
        }
        
        if(b == NULL){
            printf("\nBook not found.\n");
            sleep(1);
            continue;
        }
        
        for(i=0; i<MAX_DUP_BOOKS; i++){
            if(b->id[i] != -1){
                bookCount++;
            }
        }
        
        printf("\n\t\t\t\t\tBook(s) found");
        printf("\n\n%30s %25s %10s %15s\n", "Book", "Author", "Book ID", "Availability");
        printf("---------------------------------------------------------------------------------------------\n");
        PrintBook(b);
        printf("---------------------------------------------------------------------------------------------\n");
        
        if(bookCount>1){
            while(1){
                printf("\nMultiple entries found, please specify the Book ID: ");
                fgets(buf, 5, stdin);
                choice = atoi(buf);
                if(choice != 0){
                    for(i=0; i<MAX_DUP_BOOKS; i++){
                        if(b->id[i] == choice){
                            deleted = 1;
                            b->id[i] = -1;
                            b->status[i] = -1;
                        }
                    }
                    
                    if(deleted == 0){
                        printf("Incorrect id, try again.");
                        continue;
                    }
                    
                    printf("Book successfully deleted.\n");
                    break;
                }
            }
        }else{
            printf("\nAre you sure you want to delete this Book?(y/n) ");
            if(fgets(buf, 5, stdin) == NULL)
            return;
            buf[strlen(buf)-1] = '\0';
            if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
                DeleteBookByNode(&bookRoot, b);
                printf("\nBook successfully deleted.\n\n");
            }
        }
        
        printf("Do you want to delete another book? (y/n) ");
        if(fgets(buf, 5, stdin) == NULL)
        return;
        buf[strlen(buf)-1] = '\0';
        if(strcmp(buf, "y")==0 || strcmp(buf, "Y")==0 || strcmp(buf, "yes")==0 || strcmp(buf, "Yes")==0 || strcmp(buf, "YES")==0){
            continue;
        }else{
            break;
        }
    }
}

void LoadBooks(Book **bookRoot){
    printf("Loading Books....\n");
    FILE *f;
    char buf[100], buf2[100];
    Book b;
    int i;
    
    f = fopen(BOOKFILE, "r");
    
    if(f == NULL)
    return;
    
    while(fscanf(f, "%d ", &b.id[0]) != EOF){
        if(maxBookId < b.id[0])
        maxBookId = b.id[0];
        
        for(i=1; i<MAX_DUP_BOOKS; i++){
            fscanf(f, "%d ", &b.id[i]);
            if(maxBookId < b.id[i])
            maxBookId = b.id[i];
        }
        
        for(i=0; i<MAX_DUP_BOOKS; i++)
        fscanf(f, "%d ", &(b.status[i]));
        
        fgets(buf, sizeof(buf), f);
        sscanf(buf, "%[^\t\n]", buf2);
        b.name = (char*)malloc(strlen(buf2)+1);
        strcpy(b.name, buf2);
        
        fgets(buf, sizeof(buf), f);
        sscanf(buf, "%[^\t\n]", buf2);
        b.author = (char*)malloc(strlen(buf2)+1);
        strcpy(b.author, buf2);
        
        InsertBookInTree(bookRoot, &b, NULL);
    }
    
    fclose(f);
}

void LoadMembers(Member **memRoot){
    FILE *f;
    char buf[100], buf2[100];
    Member b;
    int i;
    
    f = fopen(MEMBERFILE, "r");
    
    if(f == NULL)
    return;
    
    while(fscanf(f, "%d ", &b.id) != EOF){
        if(maxMemId < b.id)
        maxMemId = b.id;
        
        for(i=0; i<MAX_BORROW; i++){
            fscanf(f, "%d ", &b.borrowedBookId[i]);
        }
        
        fgets(buf, sizeof(buf), f);
        sscanf(buf, "%[^\t\n]", buf2);
        b.name = (char*)malloc(strlen(buf2)+1);
        strcpy(b.name, buf2);
        
        InsertMemberInTree(memRoot, &b, NULL);
    }
    
    fclose(f);
}

void InsertBookInTree(Book **l, Book *b, Book *parent){
    Book *p;
    int i;
    
    if(*l == NULL){
        p = (Book*)malloc(sizeof(Book));
        for(i=0; i<MAX_DUP_BOOKS; i++){
            (p->id)[i] = (b->id)[i];
            (p->status)[i] = (b->status)[i];
        }
        
        p->name = (char*)malloc(strlen(b->name)+1);
        p->author = (char*)malloc(strlen(b->author)+1);
        strcpy(p->name, b->name);
        strcpy(p->author, b->author);
        
        p->left = p->right = NULL;
        p->parent = parent;
        *l = p;
        return;
    }
    
    if(strcmp(b->name, (*l)->name) < 0){
        InsertBookInTree(&((*l)->left), b, *l);
    }else{
        InsertBookInTree((&(*l)->right), b, *l);
    }
}

void InsertMemberInTree(Member **l, Member *b, Member *parent){
    Member *p;
    int i;
    
    if(*l == NULL){
        p = (Member*)malloc(sizeof(Member));
        p->id = b->id;
        for(i=0; i<MAX_BORROW; i++){
            p->borrowedBookId[i] = b->borrowedBookId[i];
        }
        
        p->name = (char*)malloc(strlen(b->name)+1);
        strcpy(p->name, b->name);
        
        p->left = p->right = NULL;
        p->parent = parent;
        *l = p;
        return;
    }
    
    if(b->id < (*l)->id){
        InsertMemberInTree(&((*l)->left), b, *l);
    }else{
        InsertMemberInTree((&(*l)->right), b, *l);
    }
}

void PrintBooks(Book *l){
    if(l != NULL){
        PrintBooks(l->left);
        PrintBook(l);
        PrintBooks(l->right);
    }
}

void PrintMembers(Member *l){
    if(l != NULL){
        PrintMembers(l->left);
        PrintMember(l);
        PrintMembers(l->right);
    }
}

void PrintBook(Book *b){
    int i;
    for(i=0; i<MAX_DUP_BOOKS; i++){
        if(b->id[i] != -1)
        printf("%30s %25s %10d %15s\n", b->name, b->author, b->id[i], (b->status[i])==1?"Available":"Not Available");
    }
}

void PrintMember(Member *b){
    int i;
    printf("%25s %5d    ", b->name, b->id);
    for(i=0; i<MAX_BORROW; i++){
        if(b->borrowedBookId[i] == -1)
        printf("%4s ", "X");
        else
            printf("%4d ", b->borrowedBookId[i]);
    }
    printf("\n");
}

void SaveBooks(Book *b, FILE **f){
    if(b != NULL){
        WriteBookToFile(f, b);
        SaveBooks(b->left, f);
        SaveBooks(b->right, f);
    }
}

void SaveMembers(Member *b, FILE **f){
    if(b != NULL){
        WriteMemberToFile(f, b);
        SaveMembers(b->left, f);
        SaveMembers(b->right, f);
    }
}

void WriteBookToFile(FILE **f, Book *b){
    char space = ' ';
    char newline = '\n';
    int i;
    
    for(i=0; i<MAX_DUP_BOOKS; i++)
    fprintf(*f, "%d ", (b->id)[i]);
    fwrite(&newline, 1, 1, *f);
    
    for(i=0; i<MAX_DUP_BOOKS; i++)
    fprintf(*f, "%d ", (b->status)[i]);
    fwrite(&newline, 1, 1, *f);
    
    int n = strlen(b->name);
    b->name[n] = '\0';
    fwrite(b->name, 1, n, *f);
    fwrite(&newline, 1, 1, *f);
    
    n = strlen(b->author);
    b->author[n] = '\0';
    fwrite(b->author, 1, n, *f);
    fwrite(&newline, 1, 1, *f);
}

void WriteMemberToFile(FILE **f, Member *b){
    char newline = '\n';
    int i;
    
    fprintf(*f, "%d", b->id);
    fwrite(&newline, 1, 1, *f);
    
    for(i=0; i<MAX_BORROW; i++)
    fprintf(*f, "%d ", b->borrowedBookId[i]);
    fwrite(&newline, 1, 1, *f);
    
    int n = strlen(b->name);
    b->name[n] = '\0';
    fwrite(b->name, 1, n, *f);
    fwrite(&newline, 1, 1, *f);
}

Book *SearchBookByName(Book *bookRoot, char name[]){
    if(bookRoot == NULL)
    return NULL;
    
    if(strcasecmp(bookRoot->name, name) == 0)
    return bookRoot;
    
    if(strcasecmp(name, bookRoot->name) < 0)
    return SearchBookByName(bookRoot->left, name);
    else
        return SearchBookByName(bookRoot->right, name);
}

Member *SearchMemberById(Member *memRoot, int id){
    if(memRoot == NULL)
    return NULL;
    
    if(memRoot->id == id)
    return memRoot;
    
    if(id < memRoot->id)
    return SearchMemberById(memRoot->left, id);
    else
        return SearchMemberById(memRoot->right, id);
}

Book *SearchBookById(Book *bookRoot, int id){
    Book *ret;
    int i;
    
    if(bookRoot == NULL)
    return NULL;
    
    for(i=0; i<MAX_DUP_BOOKS; i++){
        if(bookRoot->id[i] == id){
            return bookRoot;
        }
    }
    
    ret = SearchBookById(bookRoot->left, id);
    if(ret != NULL)
    return ret;
    else
        ret = SearchBookById(bookRoot->right, id);
    
    return ret;
}

void DeleteBookByNode(Book **bookRoot, Book *b){
    Book *parent = NULL;
    
    if(b==NULL){
        return;
    }
    
    if(b->left == NULL && b->right == NULL){
        parent = b->parent;
        if(parent == NULL){
            *bookRoot = NULL;
        }else if(strcmp(b->name, parent->name) <=0)
        parent->left = NULL;
        else
            parent->right = NULL;
        
        free(b);
        return;
    }
    
    if(b->left != NULL && b->right == NULL){
        parent = b->parent;
        if(parent == NULL){
            *bookRoot = b->left;
            (*bookRoot)->parent = parent;
        }else if(strcmp(b->name, parent->name) <= 0){
            parent->left = b->left;
            (b->left)->parent = parent;
        }else{
            parent->right = b->left;
            (b->left)->parent = parent;
        }
        
        free(b);
        return;
    }
    
    if(b->left == NULL && b->right != NULL){
        parent = b->parent;
        if(parent == NULL){
            *bookRoot = b->right;
            (*bookRoot)->parent = NULL;
        }else if(strcmp(b->name, parent->name) <= 0){
            parent->left = b->right;
            (b->right)->parent = parent;
        }else{
            parent->right = b->right;
            (b->right)->parent = parent;
        }
        
        free(b);
        return;
    }
    
    if(b->left != NULL && b->right != NULL){
        Book *temp = b->right;
        while(temp->left != NULL)
        temp = temp->left;
        strcpy(b->name, temp->name);
        if(temp == b->right){    //If temp happens to be the immediate right child of b
            b->right = temp->right;  //temp won't have any left child as it's the leftmost node in right subtree of b
            (b->right)->parent = b;
        }
        else{
            (temp->parent)->left = NULL; //if temp is not the immediate right child of b then it definitely is the left child of its parent
        }
        free(temp);
        return;
    }
}

void DeleteMemberByNode(Member **memRoot, Member *b){
    Member *parent = NULL;
    
    if(b==NULL){
        return;
    }
    
    if(b->left == NULL && b->right == NULL){
        parent = b->parent;
        if(parent == NULL){
            *memRoot = NULL;
        }else if(b->id < parent->id)
        parent->left = NULL;
        else
            parent->right = NULL;
        
        free(b);
        return;
    }
    
    if(b->left != NULL && b->right == NULL){
        parent = b->parent;
        if(parent == NULL){
            *memRoot = b->left;
            (*memRoot)->parent = parent;
        }else if(b->id < parent->id){
            parent->left = b->left;
            (b->left)->parent = parent;
        }else{
            parent->right = b->left;
            (b->left)->parent = parent;
        }
        
        free(b);
        return;
    }
    
    if(b->left == NULL && b->right != NULL){
        parent = b->parent;
        if(parent == NULL){
            *memRoot = b->right;
            (*memRoot)->parent = NULL;
        }else if(b->id < parent->id){
            parent->left = b->right;
            (b->right)->parent = parent;
        }else{
            parent->right = b->right;
            (b->right)->parent = parent;
        }
        
        free(b);
        return;
    }
    
    if(b->left != NULL && b->right != NULL){
        Member *temp = b->right;
        while(temp->left != NULL)
        temp = temp->left;
        b->id = temp->id;
        if(temp == b->right){    //If temp happens to be the immediate right child of b
            b->right = temp->right;  //temp won't have any left child as it's the leftmost node in right subtree of b
            (b->right)->parent = b;
        }
        else{
            (temp->parent)->left = NULL; //if temp is not the immediate right child of b then it definitely is the left child of its parent
        }
        free(temp);
        return;
    }
}
