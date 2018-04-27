#include "simlib.h"
#include <ctime>

using namespace std;

#define INCUBATORS 2		// 2
#define INCUBATORS_CAP 24	// 24
#define STORAGE_CAP 60		// 60
#define HENS 9				// 9
#define ROOSTER 1			// 1
#define TIME_OF_RUN 5

// minuta
const double MINUTE = 1/ (float) 60;
// hodina
const int HOUR = 1;
// den
const int DAY = 24 * HOUR;
// mesiac
const int MONTH = 30 * DAY;
// roky
const int YEAR = 12 * MONTH;

// prichod vajicka
const double EGG_ARRIVAL = (32.0 * HOUR) / HENS;
// vypadok prudu 
const int BLACKOUT_ARRIVAL = YEAR;
// prichod cloveka
const int HUMAN_ARRIVAL = 12 * HOUR;
// maximalne cakanie v uschovni
const int TIMEOUT = 10 * DAY;

// dlzka simulacia
const int T_END = TIME_OF_RUN * YEAR;

// cena 1-dnovych kurcat nosnic
const double female_price = 1.20;
// cena 1-dnovych kohutov
const double male_price = 0.20;
// cena predaneho vajicka
const double egg_price = 0.20;
// cena inkubatora (35 - 150)
const double incubator_price = 35;
// mesacne naklady na elektrinu (5)
const int electricity_price = 5;
// mesacne naklady na sliepku
const int hen_year_price = 10;
// mesacne naklady na uschovnu
const int storage_year_price = 40;
// cena kupy jednej sliepky
const double hens_price = 6.5;
// cena kupy jedneho kohuta
const int rooster_price = 5;

bool is_blackout = false;

long egg_counter; 
long sold;
long broken;
long throw_out;
long blackout_throw_out;
long chicken_male;
long chicken_female;
long failure_counter;


Queue q_Henhouse("Kurin");

Queue q_Storage("Uschovna");

Store s_Storage("Miesto v uschovni", STORAGE_CAP);

Queue q_Incubator("Incubator");

Store s_Incubator("Liahen", INCUBATORS);

Queue q_Occupation("Okupacia fronty");


/**
 * @brief      Casovac na odstranenie vajicka z uschovne
 */
class Timeout : public Event {
    Process *Id;

  public:
    Timeout(Process *p, double dt): Id(p) {
        Activate(Time+dt);
    }

	void Behavior() {
		q_Storage.GetFirst();

		Id->Leave(s_Storage);
		Id->Cancel();
		sold++;
	}
};

/**
 * @brief      Trieda pre obsadenie inkubatora
 */
class Incubator : public Process {

	 void Behavior() {
	 	// pokial je volny inkubator a je dostatocny pocet vajicok v uschovni a nie je porucha
	 	if (!s_Incubator.Full() && s_Storage.Used() >= INCUBATORS_CAP && is_blackout == false) {
	 		// obsad inkubator
	 		Enter(s_Incubator, 1);
	 		q_Occupation.Insert(this);

	 		// zoberie tolko vajiciek z uschovne, kolko sa vojde do inkubatora
	 		for (int i = 0; i < INCUBATORS_CAP; i++) {
	 			Process *egg_process = (Process *)q_Storage.GetFirst();
	 			egg_process->Leave(s_Storage);
	 			egg_process->Activate();
	 		}

	 		// Caka 22 dni
	 		Wait(22 * DAY);

	 		// vyberie vsetky vajicka z inkubatora
	 		for (int i = 0; i < INCUBATORS_CAP; i++) {
	 			Process *egg_process = (Process *)q_Incubator.GetFirst();
	 			// pokial bol proces zobudeny kvoli vypadku, vajicka sa zahadzuju
	 			if (is_blackout) {
	 				egg_process->Cancel();
	 				blackout_throw_out++;
	 			}
	 			else {
	 				egg_process->Activate();
	 			}
	 		}

	 		// Po vybrati nastava automaticky proces cistenia inkubatora 
	 		if (!is_blackout) {
				Wait(HOUR);
				// ak bol proces zobudeny pocas cistenia
	 			if (!is_blackout) {
	 				q_Occupation.GetFirst();
	 			}
 			}
 			// uvolnenie inkubatora
	 		Leave(s_Incubator);
	 		// aktivovanie procesu pre dalsiu varku
			Activate();


	 	}
	 }
};

/**
 * @brief      Porucha vypadku elektrickej energie
 */
class Blackout : public Process {

	void Behavior() {
		// zaciatok vypadku
		is_blackout = true;
		failure_counter++;

		// predanie vsetkych zasob z uschovne
		while (!q_Storage.Empty()) {
			Process *tmp = (Process *)q_Storage.GetFirst();
			tmp->Leave(s_Storage);
			tmp->Activate();
			sold++;
		}

		// vyhodenie vajiciek v inkubatoroch
		while (!q_Occupation.Empty()) {
			q_Occupation.GetFirst()->Activate();
		}

		// zabranie vsetkych inkubatorov
		Enter(s_Incubator, INCUBATORS);

		// cas bez elektriny
		Wait(Exponential(DAY));

		// po nabehnuti elektrickej energie sa spusta automaticka dezinfekcia inkubatorov 
		for (int i = 0; i < INCUBATORS; i++) {
	 		Wait(HOUR);
	 		// uvolnuje sa inkubator
	 		Leave(s_Incubator, 1);
		}
		// koniec vypadku
		is_blackout = false;
	}
};


/**
 * @brief      Proces vajicka
 */
class Egg : public Process {

	void Behavior() {

		egg_counter++;
		// h_Egg_hatching(Time);

		// vyliahnutie sliepkov v kurine
		q_Henhouse.Insert(this);

		// caka na zozbieranie
		Passivate();

		// Triedenie vajec
		// vajicko na liehnutie 
		if (Random() < 0.79) {
			// ak nieje vojde do uschovne a nieje vypadok prudu
			if (!s_Storage.Full() && is_blackout == false) {
				// zaberie miesto
				Enter(s_Storage, 1);
				// prida sa do rady
				q_Storage.Insert(this);
				// aktivuje sa proces na liahnutie vajicok
				(new Incubator)->Activate();

				Timeout *t = new Timeout(this, 10 * DAY);

				Passivate();
				delete t;

				if (is_blackout) {
					goto Egg_end;
				}

				q_Incubator.Insert(this);

				Passivate();

				// Vyberanie nevyliahnutych vajicok
				if (Random() < 0.84) {

					// Mrtve kuriatka
					if (Random() < 0.89) {
						
						// Abnormalne kuriatka
						if (Random() < 0.93) {
							// pohlavie
							if (Random() < 0.53) {
								chicken_female++;
							}
							else {
								chicken_male++;
							}
						}
						else {
							throw_out++;
						}
					}
					else {
						throw_out++;
					}
				}
				else {
					throw_out++;
				}	
			}
			// inak sa predava
			else {
				sold++;
			}
		}
		else {
			// vajicko na predanie
			if (Random() < 0.862) {
				sold++;
			}
			// rozbite vajicko ma vyhodenie
			else {
				broken++;
			}
		}

		Egg_end:;
	}
};


/**
 * @brief      Trieda modelujuca zbieranie vajicok
 */
class Collector : public Process {

	void Behavior() {
		// pokial nieje kurin prazny
		while (!q_Henhouse.Empty()) {
			// zober a vycisti vajicko
			Wait(Exponential(2 * MINUTE));
			(q_Henhouse.GetFirst())->Activate();
		}
	}
};


/**
 * @brief      Trieda pre generovanie prichodu vajicka
 */
class Egg_generator : public Event {

	void Behavior()	{
		(new Egg)->Activate();
		Activate(Time + Exponential(EGG_ARRIVAL));
	}
};

/**
 * @brief      Trieda pre generovaie prichodu zberaca do kurina
 */
class Collector_generator : public Event {

	void Behavior() {
		(new Collector)->Activate();
		Activate(Time + HUMAN_ARRIVAL);
	}
};

/**
 * @brief      Trieda pre generovanie vypadku elektrickej energie
 */
class Blackout_generator : public Event {

	void Behavior()	{
		(new Blackout)->Activate();
		Activate(Time + Exponential(BLACKOUT_ARRIVAL));
	}
};


/************** MAIN *************/
int main(int argc, char *argv[]) {

	// cas konca simulacie
	long runtime = T_END;
	RandomSeed((long ) time(NULL));

	SetOutput("stats.out");

	Init(0, runtime);


	(new Collector_generator)->Activate(Time + HUMAN_ARRIVAL);
	(new Egg_generator)->Activate();
	(new Blackout_generator)->Activate(Time + Exponential(BLACKOUT_ARRIVAL));

	Run();

	q_Henhouse.Output();
	q_Storage.Output();
	s_Storage.Output();
	s_Incubator.Output();
	
	
	Print("Celkovy pocet vajiciek: %i\n", egg_counter);
	Print("V uschovni: %i\n", q_Storage.Length());
	Print("Vyhodenych: %i\n", throw_out);
	Print("Vyhodenych poruchov: %i\n", blackout_throw_out);
	long in_system = sold + broken + throw_out + blackout_throw_out + chicken_male + chicken_female + q_Incubator.Length() + q_Storage.Length() + q_Henhouse.Length();
	Print("V systeme podla nas: %i\n", in_system);
	Print("Rozdiel: %i\n", egg_counter - in_system);


	Print("Uspesnost narodenych:  %0.2f%\n", ((chicken_male + chicken_female) / (float)((chicken_male + chicken_female) + throw_out)) * 100);
	Print("Pocet poruch: %i\n", failure_counter);

	Print("--------------------------------\n");
	Print("Rozbite v kurine: %i\n", broken);
	Print("Predanych: %i\n", sold);
	Print("Vyhodenych pri inkubacii: %i\n", throw_out + blackout_throw_out);
	Print("Vyliahnutych: %i\n", chicken_male + chicken_female);
	Print("Celkom: %i\n", egg_counter - q_Incubator.Length() - q_Storage.Length() - q_Henhouse.Length());
	Print("Rozdiel: %i\n", (egg_counter - q_Incubator.Length() - q_Storage.Length() - q_Henhouse.Length()) - broken - sold - throw_out - blackout_throw_out - chicken_male - chicken_female);

	Print("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	double female_$ = female_price * chicken_female;
	double male_$ = male_price * chicken_male;
	double egg_$ = egg_price * sold;

	Print("Zarobene na 1. dnovych nosniciach: %0.2f€\n", female_$);
	Print("Zarobene na 1. dnovych kohutov: %0.2f€\n", male_$);
	Print("Zarobene na predanych vajickach: %0.2f€\n", egg_$ );
	Print("--------------------------------\n");
	double earn = female_$ + male_$ + egg_$;
	Print("+%0.2f€\n", earn);

	double electricity_$ = (T_END / (float) MONTH) * s_Incubator.tstat.MeanValue() * electricity_price;
	double incubator_$ = (INCUBATORS - 2) * incubator_price;
	// rozdiel z povodnej konfiguracie
	double hens_$ = (HENS - 9) * hens_price;
	double rooster_$ = (ROOSTER - 1) * rooster_price;
	double hen_meal_$ = (ROOSTER + HENS) * ((T_END / (float) YEAR) * hen_year_price);
	double electricity_storage_$ = (T_END / (float) YEAR) * storage_year_price;
	Print("Naklady na elektriku inkubatorov: %0.2f€\n", electricity_$ );
	Print("Cena inkubatorov: %0.2f€\n", incubator_$ );
	Print("Cena krmiva: %0.2f€\n", hen_meal_$ );
	Print("Cena energie na uschovnu: %0.2f€\n", electricity_storage_$ );
	Print("Cena pridanych nosnic a kohutov: %0.2f€\n", hens_$ + rooster_$ );
	Print("--------------------------------\n");

	double cost = electricity_$ + incubator_$ + hen_meal_$ + electricity_storage_$ + hens_$ + rooster_$;
	Print("-%0.2f€\n", cost);

	Print("\n=%0.2f€\n", earn - cost);

}
