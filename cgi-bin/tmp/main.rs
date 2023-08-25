#[allow(dead_code)]
#[derive(Debug)]
enum UsState {
    Alabama,
    Alaska,
    // --snip--
}

#[allow(dead_code)]
enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter(UsState),
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        // unlike if that only works with booleans, match works with any type
        Coin::Penny => {
            // brackets are optional, but recommended for multiple lines
            println!("Lucky penny!");
            1 // return value
        } // comma is optional when using brackets
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter(state) => {
            // get the value inside Quarter as state
            println!("State quarter from {state:?}!");
            25
        }
    }
}

fn plus_one(x: Option<i32>) -> Option<i32> {
    match x {
        // match the Option type, not the value inside
        None => None,           // here the type is None, there is no value
        Some(i) => Some(i + 1), // i is the value inside Some
    }
    // the match expression must be exhaustive, so we have to handle all cases
    // if we try to match x without handling None, the compiler will complain
}

fn add_fancy_hat() {}
fn remove_fancy_hat() {}
#[allow(dead_code)]
fn reroll() {}

fn main() {
    let coin: Coin = Coin::Quarter(UsState::Alaska);
    println!("value: {}", value_in_cents(coin));

    let five: Option<i32> = Some(5);
    let six: Option<i32> = plus_one(five); // the return is a Option::Some
    let none: Option<i32> = plus_one(None); // the return is a Option::None

    println!("six: {:?}", six);
    println!("none: {:?}", none);

    let dice_roll = 9;
    match dice_roll {
        3 => add_fancy_hat(),
        7 => remove_fancy_hat(),
        // other = move_player(other); handling every case by storing the value in "other" and calling a function passing the value
        // _ => reroll(), handling every case calling a function, "_" throws away the value
        _ => (), // handling every other case doing nothing, using a empty tuple "()"
                 // the exception for handling every case needs to be the last
    }
}
