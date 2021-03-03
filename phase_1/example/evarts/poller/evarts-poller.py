#!/usr/bin/python
import sys
import socket
import random
import time
import base64
import os
import datetime
import struct
import string
import re
from pwn import *
import logging

logging.getLogger().setLevel(10)

ING_LINE = re.compile("([^\$]+)\$(\d+\.*\d*)")
PRICE_PARSER = re.compile("(\d+)\.(\d\d)")

CURRENT_INGREDIENTS = [
    ('meat', 250),
    ('cheese', 99),
    ('tomato', 50),
    ('lettuce', 75),
    ('pickle', 350),
    ('onion', 135)
]

SAUCES = """
alfredo marinara mustard ketchup bbq hot sriracha chipotle rooster steak
fish gravy pizza butter aioli awesome secret soy tamari teriyaki tare
mayonnaise frites curry coffee queso frosting icing chocolate
glaze strawberry cherry picante salsa vinegar oil oregano glue left right
up down adjacent
""".split()

TOPPINGS = """
bacon jalapenos spinach kale shrimp mushroom portobello egg donut seaweed
fries hashbrowns chips cookie noodles
rice salmon koi goldfish trout tuna swordfish sausage lobster crab salami
pastrami roastbeef gabagool tires candy gum newspaper cardboard bubblewrap
wires beef none top bottom orthogonal
""".split()

def wait_for_prompt():
    global s
    s.recvuntil("Enter selection:", drop=True)

def wait_for_ingredient_name():
    global s
    s.recvuntil("Provide a name for the ingredient:", drop=True)

def wait_for_ingredient_price():
    global s
    s.recvuntil("Provide a price (Enter nothing to make it free):", drop=True)

def wait_for_ingredient_price_update():
    global s
    s.recvuntil("Changing price for ingredient:", drop=True)

def wait_for_make_burger_next_ingredient():
    global s
    s.recvuntil("Next ingredient:", drop=True)

def parse_price(s):
    parsed = PRICE_PARSER.match(s)
    return int(parsed.group(1)) * 100 + int(parsed.group(2))

def parse_ingredients():
    global s
    resp = s.recvuntil("Welcome to Bob's Burgers!")
    matches = filter(None, [ING_LINE.match(i) for i in resp.split("\n")])

    ingredients = [(m.group(1).strip(),
                    parse_price(m.group(2).strip())) for m in matches]
    logging.debug("parse_ingredients: {}".format(repr(ingredients)))
    return ingredients

def test_show_ingredients():
    global s
    wait_for_prompt()
    s.send("1\n")
    got = parse_ingredients()
    logging.info(repr(got))
    logging.info(repr(CURRENT_INGREDIENTS))

    for i in CURRENT_INGREDIENTS:
        if i not in got:
            logging.error("[FAILED] Show ingredient did not succeed")
            return False


    logging.info("[SUCCESS] Show ingredient passed")
    return True

def test_add_new_ingredient():
    global s
    wait_for_prompt()
    s.send("2\n")
    wait_for_ingredient_name()
    ingredient_name = random.choice(TOPPINGS) + str(random.randint(1,999))
    logging.debug("adding ingredient " + ingredient_name)
    s.send(ingredient_name + "\n")
    wait_for_ingredient_price()
    price_pennies = random.randint(50, 25000)
    price_str = "{0}.{1:02}".format(price_pennies / 100, price_pennies % 100)
    s.send(price_str + "\n")

    new_ingredient = (ingredient_name, price_pennies)
    CURRENT_INGREDIENTS.append(new_ingredient)
    logging.debug(repr(new_ingredient))

    wait_for_prompt()
    s.send("1\n")
    if new_ingredient in parse_ingredients():
        logging.info("[SUCCESS] Add ingredient passed")
    else:
        logging.error("[FAILED] Add ingredient did not succeed")
        return False
    return True

def test_add_free_ingredient():
    global s
    wait_for_prompt()
    s.send("2\n")
    wait_for_ingredient_name()
    ingredient_name = random.choice(SAUCES) + str(random.randint(1, 999))
    logging.debug("adding free ingredient " + ingredient_name)
    s.send(ingredient_name + "\n")
    wait_for_ingredient_price()
    s.send("0.00\n")
    wait_for_prompt()

    CURRENT_INGREDIENTS.append((ingredient_name, 0))

    s.send("1\n")
    if (ingredient_name, 0) in parse_ingredients():
        logging.info("[SUCCESS] Add free ingredient passed")
    else:
        logging.error("[FAILED] Add free ingredient did not succeed")
        return False
    return True

def test_modify_addition_ingredient():
    global s
    wait_for_prompt()
    s.send("2\n")
    wait_for_ingredient_name()

    ingredient = random.choice(CURRENT_INGREDIENTS)
    CURRENT_INGREDIENTS.remove(ingredient)

    price_delta = random.randint(1, 25000)
    price_delta_str = "+{0}.{1:02}".format(price_delta / 100, price_delta % 100)

    s.send("{0}\n".format(ingredient[0]))
    wait_for_ingredient_price_update()
    s.send("{0}\n".format(price_delta_str))

    new_ingredient = (ingredient[0], ingredient[1] + price_delta)

    logging.debug(repr(new_ingredient))

    CURRENT_INGREDIENTS.append(new_ingredient)

    wait_for_prompt()
    s.send("1\n")
    if new_ingredient in parse_ingredients():
        logging.info("[SUCCESS] Add to ingredient passed")
    else:
        logging.error("[FAILED] Add to ingredient did not succeed")
        return False
    return True

def test_modify_subtract_ingredient():
    global s
    wait_for_prompt()
    s.send("2\n")
    wait_for_ingredient_name()

    ingredient = random.choice(CURRENT_INGREDIENTS)
    while ingredient[1] <=3:
        ingredient = random.choice(CURRENT_INGREDIENTS)

    CURRENT_INGREDIENTS.remove(ingredient)

    price_delta = random.randint(1, ingredient[1] - 1)
    price_delta_str = "-{0}.{1:02}".format(price_delta / 100, price_delta % 100)

    s.send("{0}\n".format(ingredient[0]))
    wait_for_ingredient_price_update()
    s.send("{0}\n".format(price_delta_str))

    new_ingredient =(ingredient[0], ingredient[1] - price_delta)

    logging.debug(repr(new_ingredient))

    CURRENT_INGREDIENTS.append(new_ingredient)

    wait_for_prompt()
    s.send("1\n")
    if new_ingredient in parse_ingredients():
        logging.info("[SUCCESS] Subtract from ingredient passed")
    else:
        logging.error("[FAILED] Subtract from ingredient did not succeed")
        return False
    return True

def test_make_burger():
    global s
    wait_for_prompt()
    s.send("3\n")
    max_ingredients = min(6, len(CURRENT_INGREDIENTS))
    ingredient_count = random.randint(1, max_ingredients)
    ingredients = random.sample(CURRENT_INGREDIENTS, ingredient_count)
    price = 0
    for i in ingredients:
        wait_for_make_burger_next_ingredient()
        s.send("{0}\n".format(i[0]))
        price += i[1]
        logging.debug("ingredient {0} total price {1}".format(repr(i), price))

    wait_for_make_burger_next_ingredient()
    s.send("q\n")

    logging.debug(s.recvuntil("The total price of this burger is $"))
    expected_price_format = "{0}.{1:02}".format(price / 100, price % 100)
    got = s.recvline()
    logging.info("expected {} got {}".format(expected_price_format, got))
    if expected_price_format in got:
        logging.info("[SUCCESS] Make burger passed")
    else:
        logging.error("[FAILED] Make burger did not succeed")
        return False
    return True

def test_admin():
    global s
    wait_for_prompt()
    s.send("5\n")
    if "Unknown menu option!" in s.recvline():
        logging.info("[SUCCESS] Admin passed")
    else:
        logging.error("[FAILED] Admin did not succeed")
        return False
    return True

def test_exit():
    global s
    wait_for_prompt()
    s.send("4\n")
    try:
        s.recvline()
    except Exception as EOFError:
        logging.info("[SUCCESS] Exit passed")
        return True
    except:
        logging.error("[FAILED] Exit did not succeed")

    return False

def run_test(length):
    logging.info('[INFO] Poller basic test started')

    ACTIVITIES = [test_show_ingredients,
                  test_add_free_ingredient,
                  test_add_new_ingredient,
                  test_modify_addition_ingredient,
                  test_modify_subtract_ingredient,
                  test_make_burger,
                  test_admin]

    FULL_INGREDIENTS_ACTIVITIES = [test_show_ingredients,
                  test_modify_addition_ingredient,
                  test_modify_subtract_ingredient,
                  test_make_burger,
                  test_admin]

    for _i in xrange(length):
        acti = random.choice(ACTIVITIES)
        if 30 <= len(CURRENT_INGREDIENTS):
            acti = random.choice(FULL_INGREDIENTS_ACTIVITIES)
        logging.info("running activity {0}".format(acti.func_name))
        got = acti()
        if not got:
            logging.info("failed :(")
            sys.exit(-1)

    logging.info("success :)")

def main():
    global s
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    random.seed(SEED)

    if 'LENGTH' in os.environ:
        LENGTH = int(os.environ['LENGTH'])
    else:
        LENGTH = random.randint(25, 75)

    print 'SEED={0}'.format(SEED)
    print 'LENGTH={0}'.format(LENGTH)

    s = remote( HOST, PORT )
    run_test(LENGTH)
    s.close()

if __name__ == '__main__':
    main()
