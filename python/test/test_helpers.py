import random
import string

def random_string(x):
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=x))

def create_random_platform():
    return {
        "name": random_string(10),
        "supports_platinum": random.choice([True, False])
    }

def create_random_game_entry(platform_id):
    return {
        "status": random.choice(["In progress", "Completed", "Not started"]),
        "rating": random.randrange(0, 5),
        "publisher": random_string(10),
        "series": random_string(15),
        "platinumed": random.choice([False, True]),
        "title": random_string(30),
        "notes": random_string(100),
        "release_year": random.randrange(1990, 2030),
        "developer": random_string(15),
        "genres": random_string(50),
        "cover_path": None,
        "price": round(random.uniform(0, 100), 2),
        "platform_id": platform_id
    }

def compare_dict_common_key_values(a, b):
    return all(a[k] == b[k] for k in a.keys() & b.keys())