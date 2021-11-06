#define  _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000
#define TRUE 1
#define FALSE 0
int NUM_OF_WORDS = 1;
typedef struct WordStruct {
    char *word;
    struct WordProbability *prob_list;
    int numOfOccur;
    int wordProbabilitySize;
    //... Add your own fields here
} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;

    int countOccurForProbability;
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

WordStruct *checkInLinkedList(LinkList *pList, char *token);

int checkProbabilityList(WordStruct *firstWordStruct, WordStruct *secondWordStruct);

void updatePrecentage(WordStruct *);

int sumOccurrences(WordStruct *pStruct);

void printLinkedList(LinkList *list);

int countFileWords(FILE *pFile);

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
    Node *temp = dictionary->first;
    for (int i = 0; i <= randomNum; i++) {
        if (i == randomNum) {
            if (strchr(temp->data->word, '.') != NULL) {
                i = 0;
                randomNum = get_random_number(dictionary->size);
                temp = dictionary->first;
                continue;
            } else {
                return temp->data;// found
            }
        }
        temp = temp->next;
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
    double randomNum = (rand() / (double) RAND_MAX) * 100.0;
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
    int countWords = 1;
    printf("%s", wordStruct->word);
    while (countWords < MAX_WORDS_IN_SENTENCE_GENERATION && strchr(wordStruct->word, '.') == NULL) {
        wordStruct = get_next_random_word(wordStruct);
        printf(" %s", wordStruct->word);
        countWords++;
    }
    printf("\n");
    return countWords;
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
    if (first_word->wordProbabilitySize == 0) { // first word to add
        first_word->prob_list = (WordProbability *) malloc(sizeof(WordProbability));
        if (!first_word->prob_list) {
            fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
            exit(EXIT_FAILURE);
        }
        first_word->prob_list[0].countOccurForProbability = 1;
        first_word->prob_list[0].word_struct_ptr = second_word;
        first_word->prob_list[0].percentage = 100.0;
        first_word->wordProbabilitySize = 1;
        updatePrecentage(first_word);
        return 1;
    }
    return checkProbabilityList(first_word, second_word); // wordprobabilitysize not 0 so its not the first time we see this word
}

void updatePrecentage(WordStruct *pStruct) { // update precentage of each word in the allocated prob_list
    int totalOccurences = sumOccurrences(pStruct);
    for (int i = 0; i < pStruct->wordProbabilitySize; i++) {
        pStruct->prob_list[i].percentage = ((double) pStruct->prob_list[i].countOccurForProbability / totalOccurences) * 100.0;
    }
}

int sumOccurrences(WordStruct *pStruct) {
    int sum = 0;
    for (int i = 0; i < pStruct->wordProbabilitySize; i++) {
        sum += pStruct->prob_list[i].countOccurForProbability;
    }
    return sum;
}

int checkProbabilityList(WordStruct *firstWordStruct, WordStruct *secondWordStruct) { // check each index in WP list if secondwordstruct is there if not realloc and add
    if (strchr(firstWordStruct->word, '.') != NULL) {
        return 0;
    }
    int WPsize = firstWordStruct->wordProbabilitySize;
    int i = 0;
    for (; i < WPsize; i++) { // check if secondword already in firstword WP
        if (strcmp(firstWordStruct->prob_list[i].word_struct_ptr->word, secondWordStruct->word) == 0) {
            firstWordStruct->prob_list[i].countOccurForProbability++;
            updatePrecentage(firstWordStruct);
            return 0;
        }
    }
    // second word is not in firstword WP so we want to realloc space
    firstWordStruct->prob_list = (WordProbability *) realloc(firstWordStruct->prob_list, sizeof(WordProbability) * (i + 1));
    if (!firstWordStruct->prob_list) {
        fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }
    firstWordStruct->prob_list[i].word_struct_ptr = secondWordStruct;
    firstWordStruct->prob_list[i].countOccurForProbability = 1;
    firstWordStruct->wordProbabilitySize++;
    updatePrecentage(firstWordStruct);
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
    int wordsCount = countFileWords(fp);
    if (words_to_read == -1) {
        words_to_read = wordsCount;
    } else if (words_to_read > wordsCount) {
        words_to_read = wordsCount;
    }
    char *line = NULL;
    size_t lineSize = 0;
    WordStruct *curr = NULL;
    WordStruct *prev = NULL;
    char *token;
    char temp[MAX_WORD_LENGTH + 1];
    while (getline(&line, &lineSize, fp) != -1) {
        int endOfLine = FALSE;
        token = strtok(line, " ");
        WordStruct *wordStruct = NULL;
        while (token != NULL && NUM_OF_WORDS < words_to_read) {
            strcpy(temp, token);
            if (temp[strlen(temp) - 1] == '\n') {
                endOfLine = TRUE;
                temp[strlen(temp) - 1] = '\0';
            }
            if ((wordStruct = checkInLinkedList(dictionary, temp)) == NULL) { // check if WS already in linked list
                createWordStruct(temp, &wordStruct); // didnt found match so create word
                if (add(dictionary, wordStruct) == 1) {
                    fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
                    exit(EXIT_FAILURE);
                }
            }
            prev = wordStruct;
            token = strtok(NULL, " ");
            // second word and on.
            while (token != NULL) {
                strcpy(temp, token);
                WordStruct *newWS = NULL;
                if (temp[strlen(temp) - 1] == '\n') {
                    endOfLine = TRUE;
                    temp[strlen(temp) - 1] = '\0';
                }
                if ((newWS = checkInLinkedList(dictionary, temp)) == NULL) {
                    createWordStruct(temp, &newWS); // didnt found match so create word
                    if (add(dictionary, newWS) == 1) {
                        fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
                        exit(EXIT_FAILURE);
                    }
                }
                curr = newWS;
                add_word_to_probability_list(prev, curr);
                if (strchr(temp, '.') != NULL) { // found end of line
                    if (endOfLine == FALSE) { // so we didnt finish to read the line and need to keep reading the words from file
                        token = strtok(NULL, " ");
                    } else { // token = NULL to end the while loop.
                        token = NULL;
                    }
                    break;
                }
                prev = curr;
                token = strtok(NULL, " ");
            }
        }
        free(line);
        line = NULL;
    }
    free(line);
    line = NULL;


}


/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary) {
    Node *node = dictionary->first;
    for (int i = 0; i < dictionary->size; i++) {
        free(node->data->prob_list);
        node->data->prob_list = NULL;
        node = node->next;
    }
    node = dictionary->first;
    for (int i = 0; i < dictionary->size; i++) {
        free(node->data->word);
        node->data->word = NULL;
        free(node->data);
        node->data = NULL;
        node = node->next;
    }
    node = dictionary->first;
    for (int i = 0; i < dictionary->size; i++) {
        Node *temp = node->next;
        free(node);
        node = NULL;
        node = temp;
    }
    free(dictionary);
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int countFileWords(FILE *file) {
    size_t lineSize = 0;
    int words = 0;
    char *line = NULL;
    char *token = NULL;
    while (getline(&line, &lineSize, file) != -1) {
        token = strtok(line, " ");
        while (token != NULL) {
            words++;
            token = strtok(NULL, " ");
        }
        free(line);
        line = NULL;
    }
    free(line);
    line = NULL;
    rewind(file);
    return words;
}

WordStruct *checkInLinkedList(LinkList *pList, char *token) { // check if the given word is in the link list already return NULL if false and the wordStruct if true.
    if (pList->first == NULL) {
        return NULL;
    }
    if (strchr(token, '\n') != NULL) {
        token[strcspn(token, "\n")] = '\0';
    }
    NUM_OF_WORDS += 1;
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
    if (!(*pStruct)) {
        fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }
    (*pStruct)->prob_list = NULL;
    (*pStruct)->word = (char *) malloc(strlen(word) + 1);
    if (!(*pStruct)->word) {
        fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }
    (*pStruct)->numOfOccur = 1;
    (*pStruct)->wordProbabilitySize = 0;
    strncpy((*pStruct)->word, word, strlen(word) + 1);
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) { // not enough arguments
        printf("Usage: Please enter parameters as followed: \n"
               "1) Seed\n"
               "2) Number of sentences to generate\n"
               "3) Path to file\n"
               "4) Optional - Number of words to read\n");
        exit(EXIT_FAILURE);
    }
    char *p;
    int seed = strtol(argv[1], &p, 10);
    srand(seed);
    FILE *tweetsFile = fopen(argv[3], "r");
    if (tweetsFile == NULL) {
        printf("Error: Cant open file");
        exit(EXIT_FAILURE);
    }
    int wordsToRead = -1;
    if (argc == 5) {
        wordsToRead = strtol(argv[4], &p, 10);
    }
    LinkList *linkList = (LinkList *) malloc(sizeof(LinkList));
    if (!linkList) {
        fprintf(stdout, "Allocation failure: couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }
    linkList->first = NULL;
    linkList->size = 0;
    fill_dictionary(tweetsFile, wordsToRead, linkList);
    int numOfTweets = strtol(argv[2], &p, 10) + 1;
    for (int i = 1; i < numOfTweets; i++) {
        printf("Tweet %d: ", i);
        generate_sentence(linkList);
    }
    free_dictionary(linkList);
    if (fclose(tweetsFile) != 0) {
        printf("Error: Cant close file");
        exit(EXIT_FAILURE);
    }
    return 0;

}
