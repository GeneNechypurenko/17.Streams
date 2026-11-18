#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <windows.h>

using namespace std;

/*
==========================================================================================================================================
                                  ДОМАШНЄ ЗАВДАННЯ 37 тема: файлові потоки. Текстові та двійкові файли.
==========================================================================================================================================
*/

// Клас даних: про файли нічого не знає
class Hero
{
private:
    string name;
    int level;
    int gold;

public:
    Hero(const string& name, int level, int gold) : name(name), level(level), gold(gold)
    {}

    Hero() : Hero("", 1, 0)
    {}

    string getName() const { return name; }
    int getLevel() const { return level; }
    int getGold() const { return gold; }

    void levelUp()
    {
        level++;
    }

    void addGold(int amount)
    {
        gold += amount;
    }

    void print() const
    {
        cout << name << ": рівень " << level << ", золото " << gold << endl;
    }
};

/*
  3. Навіщо окремий клас HeroStorage, якщо збереження можна написати прямо в Hero?
     Чому load повертає false, коли файлу немає, а save у такій ситуації кидає виняток?
*/
class HeroStorage
{
private:
    string fileName;

public:
    explicit HeroStorage(const string& fileName) : fileName(fileName)
    {}

    //  1. Що таке потік? Чому з файлом працюють тими самими <<, >> і getline, що й з консоллю?
    void save(const Hero& hero) const
    {
        ofstream file(fileName);
        if (!file.is_open())
        {
            throw runtime_error("Не вдалося відкрити " + fileName);
        }
        file << hero.getLevel() << " " << hero.getGold() << " " << hero.getName() << endl;
    }

    bool load(Hero& hero) const
    {
        ifstream file(fileName);
        if (!file.is_open())
        {
            return false;
        }
        int level;
        int gold;
        string name;
        file >> level >> gold;
        if (file.fail())                      // файл порожній або зіпсований
        {
            return false;
        }
        file >> ws;                           // пропускаємо пробіл перед ім'ям
        getline(file, name);
        hero = Hero(name, level, gold);
        return true;
    }
};

// Результат одного рівня: лише int, тому його можна записувати у двійковий файл байтами як є
struct LevelResult
{
    int stars;                                // 0–3
    int bestTime;                             // секунди
};

void printLevel(const LevelResult& result, int index)
{
    cout << "Рівень " << index + 1 << ": зірок " << result.stars << ", час " << result.bestTime << " с" << endl;
}

class LevelProgress
{
private:
    string fileName;

public:
    explicit LevelProgress(const string& fileName) : fileName(fileName)
    {}

    // Якщо файлу ще немає — створює його з порожніми результатами для count рівнів
    void createIfMissing(int count) const
    {
        ifstream check(fileName, ios::binary);
        if (check.is_open())
        {
            return;                           // файл уже є — нічого не робимо
        }
        ofstream file(fileName, ios::binary);
        if (!file.is_open())
        {
            throw runtime_error("Не вдалося створити " + fileName);
        }
        LevelResult empty = { 0, 0 };
        for (int i = 0; i < count; i++)
        {
            file.write(reinterpret_cast<const char*>(&empty), sizeof(empty));
        }
    }


    int getLevelCount() const
    {
        ifstream file(fileName, ios::binary);
        if (!file.is_open())
        {
            throw runtime_error("Не вдалося відкрити " + fileName);
        }
        file.seekg(0, ios::end);                                  // у кінець файлу
        int fileSize = static_cast<int>(file.tellg());            // позиція в кінці = розмір файлу
        return fileSize / static_cast<int>(sizeof(LevelResult));
    }

    /*
        4. Чим двійковий файл відрізняється від текстового? Чому в progress.dat можна одразу перейти до рівня 4,
           а в log.txt одразу перейти до 4-го запису не вийде? Чому героя не можна записати у файл через write і reinterpret_cast,
           як LevelResult?
    */
    LevelResult readLevel(int index) const
    {
        if (index < 0 || index >= getLevelCount())
        {
            throw out_of_range("Немає такого рівня");
        }
        ifstream file(fileName, ios::binary);
        if (!file.is_open())
        {
            throw runtime_error("Не вдалося відкрити " + fileName);
        }
        LevelResult result;
        file.seekg(index * sizeof(LevelResult), ios::beg);            // одразу на потрібний рівень
        file.read(reinterpret_cast<char*>(&result), sizeof(result));
        return result;
    }

    void saveResult(int index, const LevelResult& result) const
    {
        LevelResult old = readLevel(index);                       // заодно перевіряє index

        if (result.stars <= old.stars)
        {
            cout << "Рівень " << index + 1 << ": рекорд не побито" << endl;
            return;
        }

        fstream file(fileName, ios::in | ios::out | ios::binary);
        if (!file.is_open())
        {
            throw runtime_error("Не вдалося відкрити " + fileName);
        }
        file.seekp(index * sizeof(LevelResult), ios::beg);            // на місце старого результату
        file.write(reinterpret_cast<const char*>(&result), sizeof(result));
        cout << "Рівень " << index + 1 << ": новий рекорд!" << endl;
    }
};

//  2. Що сталося б із журналом, якби writeLog відкривала файл без ios::app? Хто і коли закриває файл, якщо не викликати close()?
void writeLog(const string& message)
{
    ofstream file("log.txt", ios::app);
    if (!file.is_open())
    {
        throw runtime_error("Не вдалося відкрити log.txt");
    }
    file << message << endl;
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    try
    {
        cout << "===== ЧАСТИНА 1 =====" << endl;
        HeroStorage storage("hero.txt");
        Hero hero;
        if (storage.load(hero))
        {
            cout << "Завантажено збереження" << endl;
        }
        else
        {
            hero = Hero("Сер Галахад", 1, 0);
            cout << "Нова гра" << endl;
        }

        hero.levelUp();
        hero.addGold(100);
        hero.print();
        storage.save(hero);

        writeLog(hero.getName() + " досяг рівня " + to_string(hero.getLevel()));

        ifstream logFile("log.txt");
        if (!logFile.is_open())
        {
            throw runtime_error("Не вдалося відкрити log.txt");
        }
        cout << endl << "Журнал:" << endl;
        string line;
        while (getline(logFile, line))
        {
            cout << line << endl;
        }

        cout << endl << "===== ЧАСТИНА 2 =====" << endl;
        LevelProgress progress("progress.dat");
        progress.createIfMissing(5);

        progress.saveResult(0, { 3, 45 });
        progress.saveResult(1, { 1, 120 });
        progress.saveResult(1, { 2, 95 });
        progress.saveResult(0, { 2, 30 });

        cout << endl;
        int count = progress.getLevelCount();
        int totalStars = 0;
        for (int i = 0; i < count; i++)
        {
            LevelResult result = progress.readLevel(i);
            printLevel(result, i);
            totalStars += result.stars;
        }
        cout << "Зірок: " << totalStars << " з " << count * 3 << endl;
    }
    catch (const exception& e)
    {
        cout << "Помилка: " << e.what() << endl;
    }

    return 0;
}

/*
 
  1. Що таке потік? Чому з файлом працюють тими самими <<, >> і getline, що й з консоллю?
  2. Що сталося б із журналом, якби writeLog відкривала файл без ios::app? Хто і коли закриває файл,
     якщо не викликати close()?
  3. Навіщо окремий клас HeroStorage, якщо збереження можна написати прямо в Hero?
     Чому load повертає false, коли файлу немає, а save у такій ситуації кидає виняток?
  4. Чим двійковий файл відрізняється від текстового? Чому в progress.dat можна одразу перейти до рівня 4,
     а в log.txt одразу перейти до 4-го запису не вийде? Чому героя не можна записати у файл через write і reinterpret_cast,
     як LevelResult?

  ВІДПОВІДІ НА ЧАСТИНУ «ПОЯСНИ» (еталон для розбору)

  1. 

  2. 

  3. 

  4. 

*/
