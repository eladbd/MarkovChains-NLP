#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000

typedef struct WordStruct {
    char *word;
    struct WordProbability *prob_list;
    int numOfOccur;
    int wordProbabilitySize;
    //... Add your own fields here
} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;

    int probabilityCount;
    double percentage;
    //... Add your own fields here
} WordProbability;

/************ LINKED LIST ************/
typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

void createWordStruct(char *word, WordStruct **pStruct);

WordStruct *checkOccurrence(LinkList *pList, char *token);

int checkProbabilityList(WordStruct *pStruct, WordStruct *secondWord);

void updatePrecentage(WordStruct *);

int occurences(WordStruct *);

void printLinkedList(LinkList *list);

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList *link_list, WordStruct *data) {
    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        return 1;
    }
    *new_node = (Node) {data, NULL};

    if (link_list->first == NULL) {
        link_list->first = new_node;
        link_list->last = new_node;
    } else {
        link_list->last->next = new_node;
        link_list->last = new_node;
    }

    link_list->size++;
    return 0;
}
/*************************************/

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number) {
    return rand() % max_number;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word(LinkList *dictionary) {
    int randomNum = get_random_number(dictionary->size);
    int i = 0;
    Node *temp = dictionary->first;
    while (i != randomNum) {
        temp = temp->next;
        i++;
        if (i == randomNum) {
            if (strchr(temp->data->word, '.') != NULL) {
                i = 0;
                randomNum = get_random_number(dictionary->size);
            } else {
                break; // found
            }
        }
    }
    return temp->data;
}

/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word(WordStruct *word_struct_ptr) {
    double totalPercentage = 0;
    double randomNum = ((double) rand() / RAND_MAX) * 100; // check casting to double
    int index = 0;
    for (int i = 0; i < word_struct_ptr->wordProbabilitySize; i++) {
        totalPercentage += word_struct_ptr->prob_list[i].percentage;
        if (totalPercentage > randomNum) {
            index = i;
            break;
        }
    }
    return word_struct_ptr->prob_list[index].word_struct_ptr;
}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList *dictionary) {
    WordStruct *wordStruct = get_first_random_word(dictionary);

}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct *first_word, WordStruct *second_word) {
    if (second_word == NULL) { // first word with dot.
        first_word->prob_list = NULL;
    }
    if (first_word->wordProbabilitySize == 0) { // first word to add
        first_word->prob_list = (WordProbability *) malloc(sizeof(WordProbability));
        first_word->prob_list[0].probabilityCount = 1;
        first_word->prob_list[0].word_struct_ptr = second_word;
        first_word->wordProbabilitySize = 1;
        updatePrecentage(first_word);
        return 1;
    }
    return checkProbabilityList(first_word, second_word);
}

void updatePrecentage(WordStruct *pStruct) {
    int totalOccurences = occurences(pStruct);
    for (int i = 0; i < pStruct->wordProbabilitySize; ++i) {
        pStruct->prob_list[i].percentage = (double) pStruct->prob_list[i].probabilityCount / totalOccurences * 100.0;
    }
}

int occurences(WordStruct *pStruct) {
    int sum = 0;
    for (int i = 0; i < pStruct->wordProbabilitySize; i++) {
        sum += pStruct->prob_list[i].probabilityCount;
    }
    return sum;
}

int checkProbabilityList(WordStruct *pStruct, WordStruct *secondWord) {
    if (strchr(pStruct->word, '.') != NULL) {
        return 0;
    }
    int pSize = pStruct->wordProbabilitySize;
    int i = 0;
    for (; i < pSize; i++) {
        if (strcmp(pStruct->prob_list[i].word_struct_ptr->word, secondWord->word) == 0) {
            pStruct->prob_list[i].probabilityCount++;
            updatePrecentage(pStruct);
            return 0;
        }
    }
    pStruct->prob_list = (WordProbability *) realloc(pStruct->prob_list, sizeof(WordProbability) * (i + 1));
    if (!pStruct->prob_list) {
        fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }
    pStruct->prob_list[i].word_struct_ptr = secondWord;
    pStruct->prob_list[i].probabilityCount = 1;
    pStruct->wordProbabilitySize++;
//    updatePrecentage(pStruct->prob_list);
    return 1;
}


/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary) {
    char *line = NULL;
    ssize_t lineSize = 0;
    size_t nread;
//    char *space = " ";
    WordStruct *curr = NULL;
    WordStruct *prev = NULL;
    char *token;
    char temp[MAX_WORD_LENGTH + 1];
    while ((nread = getline(&line, &lineSize, fp)) != -1) {
        int endOfLine = 0;
        token = strtok(line, " ");
        WordStruct *wordStruct = NULL;
        strcpy(temp, token);
//        if ((wordStruct = checkOccurrence(dictionary, temp)) == NULL) { // didnt found match in linked list
//            createWordStruct(temp, &wordStruct);
//            add(dictionary, wordStruct);
//        }
        while (token != NULL) {
            if (temp[strlen(temp) - 1] == '\n') {
                endOfLine = 1;
                temp[strlen(temp) - 1] = '\0';
            }
            if ((wordStruct = checkOccurrence(dictionary, temp)) == NULL) { // didnt found match in linked list
                createWordStruct(temp, &wordStruct);
                add(dictionary, wordStruct);
            }
            prev = wordStruct;
            token = strtok(NULL, " ");
            // second word and on.
            while (token != NULL) {
                strcpy(temp, token);
                WordStruct *newWS = NULL;
                if (temp[strlen(temp) - 1] == '\n') {
                    endOfLine = 1;
                    temp[strlen(temp) - 1] = '\0';
                }
                if ((newWS = checkOccurrence(dictionary, temp)) == NULL) { // didnt found match
                    createWordStruct(temp, &newWS);
                    add(dictionary, newWS);
                }
                curr = newWS;
                add_word_to_probability_list(prev, curr);
                if (strchr(temp, '.') != NULL) {
                    // check '/n'
                    prev = curr;
                    curr = NULL;
                    add_word_to_probability_list(prev, curr);
                    if (endOfLine == 0) {
                        token = strtok(NULL, " ");
                    } else {
                        token = NULL;
                    }
                    break;
                }
                // stopped here
                prev = curr;
                token = strtok(NULL, " ");
            }
        }
        free(line);
        line = NULL;
    }


}


/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary) {
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int countFileWords(FILE *file) {
    char c;
    int words = 0;
    while ((c = fgetc(file)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\0') {
            words++;
        }
    }
    return words;
}

WordStruct *checkOccurrence(LinkList *pList, char *token) { // check if the given word is in the link list already return -1 if false and 1 if true
    if (pList->first == NULL) {
        return NULL;
    }
    if (strchr(token, '\n') != NULL) {
        token[strcspn(token, "\n")] = '\0';
    }
    Node *tempNode = pList->first;
    for (int i = 0; i < pList->size; i++) {
        if (strcmp(tempNode->data->word, token) == 0) {
            tempNode->data->numOfOccur++;
            return tempNode->data;
        }
        tempNode = tempNode->next;
    }
    return NULL;
}

void createWordStruct(char *word, WordStruct **pStruct) {
    if (strchr(word, '\n') != NULL) {
        word[strcspn(word, "\n")] = '\0';
    }
    (*pStruct) = (WordStruct *) malloc(sizeof(WordStruct) + 1);
    (*pStruct)->prob_list = NULL;
    (*pStruct)->word = (char *) malloc(strlen(word) + 1);
    (*pStruct)->numOfOccur = 1;
    (*pStruct)->wordProbabilitySize = 0;
    strncpy((*pStruct)->word, word, strlen(word) + 1);
}

int main(int argc, char *argv[]) {
    if (argc < 4) { // not enough arguments
        printf("Usage: Please enter parameters as followed: \n1) Seed\n"
               "2) Number of sentences to generate\n"
               "3) Path to file\n"
               "4) Optional - Number of words to read\n");
        exit(EXIT_FAILURE);
    }
    int seed = argv[1];
    srand(seed);
    FILE *tweetsFile = fopen(argv[3], "r");
    if (tweetsFile == NULL) {
        printf("Error: Cant open file");
        exit(EXIT_FAILURE);
    }
    int wordsToRead = -1;
    if (argc == 5) {
        int wordsCount = countFileWords(tweetsFile);
        if (wordsCount < argv[4]) {
            wordsToRead = wordsCount;
        }
    }

    LinkList *linkList = (LinkList *) malloc(sizeof(LinkList));

    fill_dictionary(tweetsFile, wordsToRead, linkList);
    printLinkedList(linkList);

    return 0;
//    FILE *fp = fopen(argv[1], "r");
//    char *line = NULL;
//    ssize_t lineSize = 0;
//    size_t nread;
//    char *space = " ";
//    char *curr;
//    char *prev;
//    int counter = 0;
//    while (getline(&line, &lineSize, fp) != -1) {
//        char *token;
//        token = strtok(line, space);
//        prev = token;
////        printf("%s\n", prev);
//        counter++;
//        while (token != NULL) {
//            token = strtok(NULL, space);
//            // second word.
//            while (token != NULL) {
//                counter++;
//                curr = token;
//                printf("prev is: |%s| current is : |%s|\n", prev, curr);
//                if (strchr(token, '.') != NULL) {//
//                    prev = curr;
//                    curr = NULL;
////                    printf("prev is: |%s| current is : |%s|\n", prev, curr);
//                    break;
//                }
//                prev = curr;
////            wordStruct->prob_list->word_struct_ptr;
//                token = strtok(NULL, space);
//
//            }
//            free(line);
//            line = NULL;
//        }
//
//    }
//    printf("total words : %d\n", counter);
//    return 0;
//

}

void printLinkedList(LinkList *list) {
    int i = 0;
    Node *tempNode = list->first;
    while (tempNode != NULL) {
        printf("%d : %s : occur: %d\n", i, tempNode->data->word, tempNode->data->numOfOccur);
        i++;
        tempNode = tempNode->next;
    }
}
