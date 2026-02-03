#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_WORD_LENGTH 1000
#define NUM_OF_TERMS 6
#define NUM_OF_QUESTIONS 38

#define FULLY_DISAGREE 0 // FD
#define DISAGREE 1 // D
#define PARTIALLY_DISAGREE 2 // PD
#define PARTIALLY_AGREE 3 // A
#define AGREE 4 // PA
#define FULLY_AGREE 5 // FA

// ====<<HELPER METHODS>>====
void store_bits(char *bits, bool test_bits[4]);
void store_questions(char *q, char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH]);
void store_order(char *o, bool order[NUM_OF_QUESTIONS]);
void store_likert_terms(char *lt, char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH]);

void store_respondent_info(char *r, char answers[][NUM_OF_QUESTIONS][MAX_WORD_LENGTH], int *num_of_respondents);

void record_answers(bool order[NUM_OF_QUESTIONS], char answers[][NUM_OF_QUESTIONS][MAX_WORD_LENGTH], float r_values[5][NUM_OF_QUESTIONS], float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS], char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH], int num_of_respondents);
void avg_per_respondent(float r_averages[5][5], float r_values[5][NUM_OF_QUESTIONS]);
void find_section_avg(float s_averages[5], float r_averages[5][5], int num_of_respondents);

void print_title(int num_of_respondents);
void print_questions(char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH], float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS], char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH]);
void print_respondent_avg(float r_averages[5][5]);
void print_section_avg(float s_averages[5]);
void print_output(bool test_bits[4], char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH], bool order[NUM_OF_QUESTIONS], char answers[5][NUM_OF_QUESTIONS][MAX_WORD_LENGTH], float r_values[5][NUM_OF_QUESTIONS], float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS], char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH], float r_averages[5][5], float s_averages[5], int num_of_respondents);

int main(int argc, char *argv[])
{
	int num_of_respondents = 0;

	// Store test bits for determining future output
	// 0 = Off/False, 1 = On/True
	bool test_bits[4];

	char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH];

	char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH];

	// Stores all converted answers of each individual respondent per question
	float r_values[5][NUM_OF_QUESTIONS];

	// Stores relative frequency of each likert term for each question
	float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS];

	// Stores average scores per respondent (5 total) for each section (5 total)
	float r_averages[5][5];

	// Stores average score for each section (5 total), combining all respondent scores
	float s_averages[5];

	// Stores Direct/Reverse values of each question
	bool order[NUM_OF_QUESTIONS];

	// Stores answers (chosen likert terms) of each individual respondent
	// 5 = num_of_respondents
	char answers[5][NUM_OF_QUESTIONS][MAX_WORD_LENGTH];

	char input[9500]; // ~ 9200 characters max in input files
	char *cur_section;

	bool bits_checked = false;
	bool q_checked = false;
	bool order_checked = false;
	bool terms_checked = false;

	while (fgets(input, sizeof(input), stdin))
	{
		// This divides the input into 5 sections:
		// Test Bits -> Questions -> Order (Direct or Reverse) -> Likert Terms (we skip processing this) -> Respondents
		cur_section = input;
		cur_section = strtok(input, "\n");
		
		// Ignore test comments in each file
		if (cur_section[0] != '#')
		{
			if (!bits_checked)
			{
				// Test bits always come first
				store_bits(cur_section, test_bits);
				bits_checked = true;
			}
			else if (!q_checked)
			{
				store_questions(cur_section, questions);
				q_checked = true;
			}
			else if (!order_checked)
			{
				store_order(cur_section, order);
				order_checked = true;
			}
			// Skip operations on likert terms
			else if (!terms_checked)
			{
				store_likert_terms(cur_section, likert_terms);
				terms_checked = true;
			}
			else
			{
				// Last part of file is storing respondent info
				// in01.txt will not reach this space

				// Loop through each respondent section until EOF
				store_respondent_info(cur_section, answers, &num_of_respondents);
			}
		}
		// Iterates to next section
		cur_section = strtok(NULL, "\n");
	}

	print_title(num_of_respondents);
	print_output(test_bits, questions, order, answers, r_values, q_values, likert_terms, r_averages, s_averages, num_of_respondents);
	return 0;
}

// Prints title sequence of desired output
void print_title(int num_of_respondents)
{
	printf("Examining Science and Engineering Students' Attitudes Towards Computer Science\n");
	printf("SURVEY RESPONSE STATISTICS\n\n");
	printf("NUMBER OF RESPONDENTS: %i\n", num_of_respondents);
}

// Save beginning test bits to an array to determine future output
// ***0 = FALSE; 1 = TRUE***
void store_bits(char *bits, bool test_bits[4])
{
	char *bit = strtok(bits, ",");

	for (int i = 0; i < 4; i++)
	{
		if (*bit == '0')
		{
			test_bits[i] = false;
		}
		else if (*bit == '1')
		{
			test_bits[i] = true;
		}
		bit = strtok(NULL, ",");
	}
}

// Save survey questions to an array for future output
void store_questions(char *q, char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH]) // q = questions before separated by ";"
{
	char *cur_question = strtok(q, ";");
	for (int i = 0; i < NUM_OF_QUESTIONS; i++)
	{
		strcpy(questions[i], cur_question);
		cur_question = strtok(NULL, ";");
	}
}

// Save order to boolean array proportional to each question
// ***DIRECT = TRUE; REVERSE = FALSE***
// Here, we use the length of "Direct" and "Reverse" to avoid hardcoding strings
void store_order(char *o, bool order[NUM_OF_QUESTIONS]) // o = direct/reverse section before separated by ";"
{
	char *d_or_r = strtok(o, ";");

	for (int i = 0; i < NUM_OF_QUESTIONS; i++)
	{
		if (strlen(d_or_r) == 6)
		{
			order[i] = true;
		}
		else if (strlen(d_or_r) == 7)
		{
			order[i] = false;
		}
		d_or_r = strtok(NULL, ";");
	}
}

void store_likert_terms(char *lt, char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH]) // lt = likert terms before separated by ","
{
	char *cur_term = strtok(lt, ",");
	for (int i = 0; i < NUM_OF_TERMS; i++)
	{
		strcat(likert_terms[i], cur_term);
		cur_term = strtok(NULL, ",");
	}
}

// Keeps track of respondents and saves their answers to each question to an array proportional to questions
void store_respondent_info(char *r, char answers[][NUM_OF_QUESTIONS][MAX_WORD_LENGTH], int *num_of_respondents) // r = respondent info before separated by ","
{
	char *cur_answer = strtok(r, ",");

	// This skips the first 3 strings of info not being used
	cur_answer = strtok(NULL, ",");
	cur_answer = strtok(NULL, ",");
	cur_answer = strtok(NULL, ",");

	for (int i = 0; i < NUM_OF_QUESTIONS; i++)
	{
		// For each question, save current respondent's answer to an array
		strcpy(answers[*num_of_respondents][i], cur_answer);
		cur_answer = strtok(NULL, ",");
	}
	
	// Keep track of each respondent
	(*num_of_respondents)++;
}

// Records and converts the answers of each respondent from a likert term
// to a numeric value based on the order of each question (direct or reverse).
void record_answers(bool order[NUM_OF_QUESTIONS], char answers[][NUM_OF_QUESTIONS][MAX_WORD_LENGTH], float r_values[5][NUM_OF_QUESTIONS], float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS], char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH], int num_of_respondents)
{	
	// Determine percentage distribution for each respondent
	int percent = 100 / num_of_respondents;

	for (int i = 0; i < 5; i++) // For each respondent...
	{
		for (int j = 0; j < NUM_OF_QUESTIONS; j++) // For each question/answer...
		{
			// Check order of question to determine scoring system for each question
			if (order[j]) // If direct, then 1(FD), 2(D), 3(PD), 4(PA), 5(A), 6(FA) 
			{
				if (strcmp(answers[i][j], likert_terms[FULLY_DISAGREE]) == 0)
				{
					// Records individual respondent's score per question
					r_values[i][j] += 1;

					// Records likert term percentage for each question
					q_values[j][FULLY_DISAGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[DISAGREE]) == 0)
				{
					r_values[i][j] += 2;
					q_values[j][DISAGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[PARTIALLY_DISAGREE]) == 0)
				{
					r_values[i][j] += 3;
					q_values[j][PARTIALLY_DISAGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[PARTIALLY_AGREE]) == 0)
				{
					r_values[i][j] += 4;
					q_values[j][PARTIALLY_AGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[AGREE]) == 0)
				{
					r_values[i][j] += 5;
					q_values[j][AGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[FULLY_AGREE]) == 0)
				{
					r_values[i][j] += 6;
					q_values[j][FULLY_AGREE] += percent;
				}
			}
			else if (!order[j]) // If reverse, then 6(FD), 5(D), 4(PD), 3(PA), 2(A), 1(FA)
			{
				if (strcmp(answers[i][j], likert_terms[FULLY_DISAGREE]) == 0)
				{
					r_values[i][j] += 6;
					q_values[j][FULLY_DISAGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[DISAGREE]) == 0)
				{
					r_values[i][j] += 5;
					q_values[j][DISAGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[PARTIALLY_DISAGREE]) == 0)
				{
					r_values[i][j] += 4;
					q_values[j][PARTIALLY_DISAGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[PARTIALLY_AGREE]) == 0)
				{
					r_values[i][j] += 3;
					q_values[j][PARTIALLY_AGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[AGREE]) == 0)
				{
					r_values[i][j] += 2;
					q_values[j][AGREE] += percent;
				}
				else if (strcmp(answers[i][j], likert_terms[FULLY_AGREE]) == 0)
				{
					r_values[i][j] += 1;
					q_values[j][FULLY_AGREE] += percent;
				}
			}
		}
	}
}

// Finds each individual respondent's average score per question section (i.e. C, I, G, U, P)
void avg_per_respondent(float r_averages[5][5], float r_values[5][NUM_OF_QUESTIONS])
{
	// ==<INDEX VALUES PER SECTION>==
	// C = 0-7 (8 Questions)
	// I = 8-17 (10 Questions)
	// G = 18-27 (10 Questions)
	// U = 28-33 (6 Questions)
	// P = 34-37 (4 Questions)

	for (int i = 0; i < 5; i++) // For each respondent...
	{
		for (int j = 0; j < NUM_OF_QUESTIONS; j++) // For each question score...
		{
			// Section C:
			if (j <= 7)
			{
				r_averages[i][0] += r_values[i][j];
			}
			// Section I:
			else if (j <= 17)
			{
				r_averages[i][1] += r_values[i][j];
			}
			// Section G:
			else if (j <= 27)
			{
				r_averages[i][2] += r_values[i][j];
			}
			// Section U:
			else if (j <= 33)
			{
				r_averages[i][3] += r_values[i][j];
			}
			// Section P:
			else if (j <= 37)
			{
				r_averages[i][4] += r_values[i][j];
			}
		}
	}

	for (int i = 0; i < 5; i++)
	{
		// Divide by total number of questions per section to find averages
		r_averages[i][0] /= 8;
		r_averages[i][1] /= 10;
		r_averages[i][2] /= 10;
		r_averages[i][3] /= 6;
		r_averages[i][4] /= 4;
	}
}

// Find combined respondents' average scores for each question section
void find_section_avg(float s_averages[5], float r_averages[5][5], int num_of_respondents)
{
	// i = Section Index
	// j = Respondent Index
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			s_averages[i] += r_averages[j][i];
		}
	}

	for (int i = 0; i < 5; i++)
	{
		// Divide by # of respondents to find average
		s_averages[i] /= num_of_respondents;
	}
}

// Prints all survey questions with answers from respondents
void print_questions(char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH], float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS], char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH])
{
	printf("\nFOR EACH QUESTION BELOW, RELATIVE PERCENTUAL FREQUENCIES ARE COMPUTED FOR EACH LEVEL OF AGREEMENT\n");
	for (int i = 0; i < NUM_OF_QUESTIONS; i++)
	{
		printf("\n%s\n", questions[i]);
		printf("%.2f: %s\n", q_values[i][FULLY_DISAGREE], likert_terms[FULLY_DISAGREE]);
		printf("%.2f: %s\n", q_values[i][DISAGREE], likert_terms[DISAGREE]);
		printf("%.2f: %s\n", q_values[i][PARTIALLY_DISAGREE], likert_terms[PARTIALLY_DISAGREE]);
		printf("%.2f: %s\n", q_values[i][PARTIALLY_AGREE], likert_terms[PARTIALLY_AGREE]);
		printf("%.2f: %s\n", q_values[i][AGREE], likert_terms[AGREE]);
		printf("%.2f: %s\n", q_values[i][FULLY_AGREE], likert_terms[FULLY_AGREE]);
	}
}

// Prints average score of each section for each individual respondent
// Section Indices: C = 0, I = 1, G = 2, U = 3, P = 4
void print_respondent_avg(float r_averages[5][5])
{
	printf("\nSCORES FOR ALL THE RESPONDENTS\n\n");

	// i = Respondent index
	for (int i = 0; i < 5; i++)
	{
		printf("C:%.2f,", r_averages[i][0]);
		printf("I:%.2f,", r_averages[i][1]);
		printf("G:%.2f,", r_averages[i][2]);
		printf("U:%.2f,", r_averages[i][3]);
		printf("P:%.2f\n", r_averages[i][4]);
	}
}

// Prints collective score average of each section (respondents answers are combined)
// Section Indices: C = 0, I = 1, G = 2, U = 3, P = 4
void print_section_avg(float s_averages[5])
{
	printf("\nAVERAGE SCORES PER RESPONDENT\n\n");
	printf("C:%.2f,", s_averages[0]);
	printf("I:%.2f,", s_averages[1]);
	printf("G:%.2f,", s_averages[2]);
	printf("U:%.2f,", s_averages[3]);
	printf("P:%.2f\n", s_averages[4]);
}

// Print desired output based on our test bits
void print_output(bool test_bits[4], char questions[NUM_OF_QUESTIONS][MAX_WORD_LENGTH], bool order[NUM_OF_QUESTIONS], char answers[5][NUM_OF_QUESTIONS][MAX_WORD_LENGTH], float r_values[5][NUM_OF_QUESTIONS], float q_values[NUM_OF_QUESTIONS][NUM_OF_TERMS], char likert_terms[NUM_OF_TERMS][MAX_WORD_LENGTH], float r_averages[5][5], float s_averages[5], int num_of_respondents)
{
	// ====<BIT DEFINITIONS>====
	// 1, _, _, _ = 1ST BIT -> print questions (no answers) - reserved for in01.txt
	// _, 1, _, _ = 2ND BIT -> print questions (with answers)
	// _, _, 1, _ = 3RD BIT -> print scores for all respondents
	// _, _, _, 1 = 4TH BIT -> print respondents' average scores for each question section

	// All input files besides in01 will have answers
	if (!test_bits[0])
	{
		record_answers(order, answers, r_values, q_values, likert_terms, num_of_respondents);
	}
	if (test_bits[0] || test_bits[1]) // print_questions() can handle blank answers (in01.txt)
	{
		print_questions(questions, q_values, likert_terms);
	}
	if (test_bits[2])
	{
		avg_per_respondent(r_averages, r_values);
		print_respondent_avg(r_averages);
	}
	if (test_bits[3])
	{
		find_section_avg(s_averages, r_averages, num_of_respondents);
		print_section_avg(s_averages);
	}
}
