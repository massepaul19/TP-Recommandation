import random
import time

# Tes 50 user_ids initiaux (exemple)
user_ids_50 = [
    249, 83, 7, 64, 101, 10, 103, 104, 17, 89,
    567, 123, 5, 67, 98, 400, 12, 90, 65, 24,
    302, 403, 205, 354, 306, 307, 305, 318, 408, 411,
    420, 409, 415, 450, 451, 204, 321, 312, 209, 401,
    402, 322, 301, 350, 410, 500, 501, 502, 503, 504
]

# Tes 40 article_ids initiaux (exemple)
article_ids_40 = [
    1087, 1134, 1231, 1042, 1088, 1135, 1232, 1043, 1564, 1897,
    1356, 1110, 1450, 1150, 1175, 1200, 1225, 1300, 1325, 1340,
    1360, 1380, 1400, 1420, 1440, 1460, 1480, 1500, 1520, 1540,
    1550, 1570, 1580, 1590, 1600, 1610, 1620, 1630, 1640, 1650
]

# Tes 25 catégories initiales (exemple)
categories_25 = [
    21, 34, 43, 32, 22, 35, 44, 50, 20, 33,
    47, 36, 40, 23, 25, 27, 29, 31, 37, 39,
    41, 42, 45, 46, 48
]

def random_timestamp(start_year=2019, end_year=2023):
    start = time.mktime(time.strptime(f'1 Jan {start_year}', '%d %b %Y'))
    end = time.mktime(time.strptime(f'31 Dec {end_year}', '%d %b %Y'))
    return int(random.randint(start, end))

def generate_transactions(n=1000, filename="transactions.txt"):
    with open(filename, "w") as f:
        for _ in range(n):
            user = random.choice(user_ids_50)
            article = random.choice(article_ids_40)
            category = random.choice(categories_25)
            rating = round(random.uniform(1.5, 5.0), 1)
            timestamp = random_timestamp()
            f.write(f"{user} {article} {category} {rating} {timestamp}\n")

def generate_user_counts(filename="user_counts.txt"):
    with open(filename, "w") as f:
        for user_id in user_ids_50:
            count = random.randint(1, 60)
            f.write(f"{user_id};{count}\n")

if __name__ == "__main__":
    generate_transactions(1000)
    generate_user_counts()

