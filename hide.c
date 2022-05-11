#include <stdlib.h>
#include <stdio.h>

// Function name : hide_file
// Input: char *file_source, char *file_destination, char *tag
// Output: None
// Description: Hide file in file_destination from file_source using sed command using a tag as a delimiter
void hide_file(char *file_to_hide, char *file, char *tag) {
    char command[128];
    // put the tag in the file
    snprintf(command, sizeof(command), "sed -i -e '$a%s' %s", tag, file);
    system(command);

    // hide the file to hide in the file source
    snprintf(command, sizeof(command), "cat %s >> %s", file_to_hide, file);
    system(command);

}


// Function name : extract_file
// Input: char *file_source, char *file_destination, char *tag
// Output: None
// Description: Extract file from file_source to file_destination using sed command using a tag as a delimiter
void extract_file(char *file, char *file_destination, char *tag) {
    char command[128];

    // extract the file to hide from the file source and put it in the file destination
    snprintf(command, sizeof(command), "sed -ne '/^%s$/{:a' -e 'n;p;ba' -e '}' %s >> %s ", tag, file,file_destination);
    system(command);

    // remove the tag from the file source and all the data of the file to hide in
    snprintf(command, sizeof(command), "sed -i '/%s/Q' %s", tag, file);
    system(command);
}
/*
int main() {
    char * tag = "#GiveMeUwU";
    char * tag2 = "#ElonMusk";

    //hide_file("data/lave.png","data/test.mp3",tag);
    //hide_file("data/sunrise.png","data/test.mp3",tag2);
    extract_file("test.mp3","sunrise.png",tag2);
    extract_file("test.mp3","lave.png",tag);

    return 0;
}
 */
