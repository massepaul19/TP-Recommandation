import random
import time

# Tes 50 user_ids modifiés

user_ids_50 = [
    157, 92, 15, 73, 128, 19, 156, 198, 26, 94,
    612, 187, 8, 81, 117, 445, 23, 103, 79, 31,
    389, 467, 278, 391, 334, 376, 398, 356, 478, 489,
    501, 456, 482, 523, 547, 267, 365, 349, 234, 434,
    443, 378, 325, 402, 467, 578, 589, 601, 612, 634
]

# Tes 40 article_ids modifiés
article_ids_40 = [
    1156, 1203, 1298, 1089, 1167, 1214, 1301, 1078, 1643, 1976,
    1423, 1189, 1534, 1234, 1259, 1284, 1309, 1378, 1403, 1428,
    1445, 1467, 1489, 1512, 1534, 1556, 1578, 1612, 1634, 1656,
    1678, 1698, 1719, 1742, 1763, 1784, 1806, 1827, 1848, 1869
]

# Tes 25 catégories modifiées

categories_25 = [
    28, 41, 52, 39, 29, 42, 53, 61, 27, 40,
    54, 43, 47, 30, 32, 34, 36, 38, 44, 46,
    48, 49, 52, 53, 55
]

def random_timestamp(start_year=2020, end_year=2024):
    start = time.mktime(time.strptime(f'1 Jan {start_year}', '%d %b %Y'))
    end = time.mktime(time.strptime(f'31 Dec {end_year}', '%d %b %Y'))
    return int(random.randint(start, end))

def generate_transactions(n=1500, filename="transactions_generées.txt"):
    with open(filename, "w") as f:
        for _ in range(n):
            user = random.choice(user_ids_50)
            article = random.choice(article_ids_40)
            category = random.choice(categories_25)
            rating = round(random.uniform(1.0, 5.0), 1)
            timestamp = random_timestamp()
            f.write(f"{user} {article} {category} {rating} {timestamp}\n")

if __name__ == "__main__":
    generate_transactions(3000)
    print("Fichiers générés avec succès !")
    print("- transactions_generées (transactions)")
    print("- Aller maintenant copier dans data/donnees.txt !")
    print("- Justification : Afin d'éviter que nos données soient écrasées ! ")
