#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h> // For sleep function

#define MAX_ROOMS 50
#define MAX_PARKING 30
#define MAX_GUESTS 100
#define MAX_BOOKINGS 200
#define MAX_STAFF 20
#define MAX_NAME 50
#define MAX_ID 15
#define MAX_PASS 20
#define FACILITIES 5
#define MAX_SERVICES 10
#define MAX_MAINTENANCE 20
#define MAX_FEEDBACK 50
#define MAX_INVENTORY 100
#define MAX_EVENTS 30
#define MAX_SCHEDULES 50
#define MAX_BILLS 200
#define DISCOUNT_THRESHOLD 1000
#define LOYALTY_POINTS_RATE 10 // Points per $100 spent
#define MAX_LOGS 1000

// Date structure
typedef struct
{
    int day, month, year;
} Date;

// Guest structure with linked list for history
typedef struct Guest
{
    char id[MAX_ID];
    char name[MAX_NAME];
    char contact[15];
    char email[50];
    int loyalty_points;
    char preferences[100];
    struct Guest *next;
} Guest;

// Room structure
typedef struct
{
    int room_no;
    int type;   // 1: Standard, 2: Deluxe, 3: Suite
    int status; // 0: Available, 1: Booked, 2: Maintenance
    float price;
    int floor;
    int capacity;
    char features[100];
} Room;

// Parking structure with linked list for waiting queue
typedef struct ParkingSlot
{
    int slot_no;
    int status; // 0: Available, 1: Occupied, 2: Reserved
    char vehicle[20];
    char guest_id[MAX_ID];
    struct ParkingSlot *next;
} ParkingSlot;

// Facility structure
typedef struct
{
    int facility_id;
    char name[30];
    int status;
    float booking_fee;
    int adjacent[FACILITIES];
} Facility;

// Booking structure with tree node
typedef struct BookingNode
{
    char booking_id[10];
    char guest_id[MAX_ID];
    int room_no;
    int parking_slot;
    int facility_id;
    Date check_in;
    Date check_out;
    float total;
    int status; // 0: Upcoming, 1: Active, 2: Completed, 3: Cancelled
    struct BookingNode *left;
    struct BookingNode *right;
} BookingNode;

// User structure for staff/admin
typedef struct User
{
    char id[MAX_ID];
    char name[MAX_NAME];
    char pass[MAX_PASS];
    char role[10]; // "staff" or "admin"
    char department[20];
    int access_level;
} User;

// Room service structure
typedef struct
{
    int service_id;
    int room_no;
    char items[100];
    char time[10];
    char status[20]; // "Pending", "In Progress", "Completed"
    float charge;
} RoomService;

// Maintenance request structure
typedef struct
{
    int request_id;
    int room_no;
    char issue[100];
    char priority[10]; // "Low", "Medium", "High"
    char status[20];   // "Open", "In Progress", "Resolved"
    Date report_date;
} MaintenanceRequest;

// Feedback structure
typedef struct
{
    int feedback_id;
    char guest_id[MAX_ID];
    char comment[200];
    int rating; // 1-5
    Date date;
} Feedback;

// Inventory item structure
typedef struct
{
    int item_id;
    char name[50];
    int quantity;
    float unit_price;
    char category[30]; // "Food", "Housekeeping", "Maintenance", etc.
    int reorder_level;
} InventoryItem;

// Event structure
typedef struct
{
    int event_id;
    char name[50];
    Date date;
    char time[10];
    int facility_id;
    char organizer[MAX_NAME];
    int capacity;
    float cost;
    int status; // 0: Planned, 1: Ongoing, 2: Completed
} Event;

// Staff schedule structure
typedef struct
{
    int schedule_id;
    char staff_id[MAX_ID];
    Date date;
    char shift[20]; // "Morning", "Evening", "Night"
    char task[50];
} StaffSchedule;

// Billing structure
typedef struct
{
    int bill_id;
    char guest_id[MAX_ID];
    char booking_id[10];
    float room_charges;
    float service_charges;
    float facility_charges;
    float taxes;
    float total;
    int status; // 0: Unpaid, 1: Paid
    Date issue_date;
} Bill;

// System log structure
typedef struct
{
    int log_id;
    char user_id[MAX_ID];
    char action[100];
    Date date;
    char time[10];
} SystemLog;

// Global variables
Room rooms[MAX_ROOMS];
ParkingSlot *parking_slots = NULL;
ParkingSlot *parking_waitlist = NULL;
Facility facilities[FACILITIES];
Guest *guest_list = NULL;
BookingNode *booking_tree = NULL;
User users[MAX_STAFF + 1];
RoomService services[MAX_SERVICES];
MaintenanceRequest maintenance[MAX_MAINTENANCE];
Feedback feedbacks[MAX_FEEDBACK];
InventoryItem inventory[MAX_INVENTORY];
Event events[MAX_EVENTS];
StaffSchedule schedules[MAX_SCHEDULES];
Bill bills[MAX_BILLS];
SystemLog logs[MAX_LOGS];
int guest_count = 0, booking_count = 0, user_count = 0;
int service_count = 0, maintenance_count = 0, feedback_count = 0;
int inventory_count = 0, event_count = 0, schedule_count = 0, bill_count = 0;
int log_count = 0;
float total_revenue = 0;

// ======================== UTILITY FUNCTIONS ========================

// Generate random ID
void generate_id(char *id, int length)
{
    int i;
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (i = 0; i < length; ++i)
    {
        id[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    id[length] = '\0';
}

// Validate date
int is_valid_date(Date d)
{
    if (d.year < 2023 || d.year > 2100)
        return 0;
    if (d.month < 1 || d.month > 12)
        return 0;
    if (d.day < 1 || d.day > 31)
        return 0;
    if ((d.month == 4 || d.month == 6 || d.month == 9 || d.month == 11) && d.day > 30)
        return 0;
    if (d.month == 2)
    {
        int leap = (d.year % 4 == 0 && (d.year % 100 != 0 || d.year % 400 == 0));
        if (d.day > (28 + leap))
            return 0;
    }
    return 1;
}

// Compare dates
int compare_dates(Date d1, Date d2)
{
    if (d1.year != d2.year)
        return d1.year - d2.year;
    if (d1.month != d2.month)
        return d1.month - d2.month;
    return d1.day - d2.day;
}

// Calculate days between dates
int date_diff(Date d1, Date d2)
{
    struct tm tm1 = {0, 0, 0, d1.day, d1.month - 1, d1.year - 1900};
    struct tm tm2 = {0, 0, 0, d2.day, d2.month - 1, d2.year - 1900};
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return difftime(t2, t1) / (60 * 60 * 24);
}

// Get current date
Date get_current_date()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    Date current = {tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900};
    return current;
}

// Get current time
void get_current_time(char *time_str)
{
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(time_str, 10, "%H:%M", tm);
}

// Log system activity
void log_activity(char *user_id, char *action)
{
    if (log_count >= MAX_LOGS)
        return;
    logs[log_count].log_id = log_count + 1;
    strcpy(logs[log_count].user_id, user_id);
    strcpy(logs[log_count].action, action);
    logs[log_count].date = get_current_date();
    get_current_time(logs[log_count].time);
    log_count++;
}

// ======================== LINKED LIST OPERATIONS ========================

// Add guest
void add_guest_to_list(Guest **head, Guest new_guest)
{
    Guest *new_node = (Guest *)malloc(sizeof(Guest));
    strcpy(new_node->id, new_guest.id);
    strcpy(new_node->name, new_guest.name);
    strcpy(new_node->contact, new_guest.contact);
    strcpy(new_node->email, new_guest.email);
    new_node->loyalty_points = 0;
    strcpy(new_node->preferences, "");
    new_node->next = *head;
    *head = new_node;
    guest_count++;
}

// Find guest
Guest *find_guest_in_list(Guest *head, char *id)
{
    Guest *current = head;
    while (current != NULL)
    {
        if (strcmp(current->id, id) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

// Add parking slot
void add_parking_slot(ParkingSlot **head, int slot_no)
{
    ParkingSlot *new_node = (ParkingSlot *)malloc(sizeof(ParkingSlot));
    new_node->slot_no = slot_no;
    new_node->status = 0;
    strcpy(new_node->vehicle, "");
    strcpy(new_node->guest_id, "");
    new_node->next = *head;
    *head = new_node;
}

// Add to parking waitlist
void add_to_waitlist(ParkingSlot **head, char *vehicle, char *guest_id)
{
    ParkingSlot *new_node = (ParkingSlot *)malloc(sizeof(ParkingSlot));
    new_node->slot_no = -1;
    new_node->status = 1;
    strcpy(new_node->vehicle, vehicle);
    strcpy(new_node->guest_id, guest_id);
    new_node->next = *head;
    *head = new_node;
}

// Remove from waitlist
ParkingSlot *remove_from_waitlist(ParkingSlot **head)
{
    if (*head == NULL)
        return NULL;
    ParkingSlot *temp = *head;
    *head = (*head)->next;
    return temp;
}

// ======================== BINARY TREE OPERATIONS ========================

// Create new booking node
BookingNode *new_booking_node(char *guest_id, int room_no, int parking_slot,
                              int facility_id, Date ci, Date co, float total)
{
    BookingNode *node = (BookingNode *)malloc(sizeof(BookingNode));
    generate_id(node->booking_id, 6);
    strcpy(node->guest_id, guest_id);
    node->room_no = room_no;
    node->parking_slot = parking_slot;
    node->facility_id = facility_id;
    node->check_in = ci;
    node->check_out = co;
    node->total = total;
    node->status = 0;
    node->left = node->right = NULL;
    booking_count++;
    return node;
}

// Insert booking
BookingNode *insert_booking(BookingNode *node, BookingNode *new_node)
{
    if (node == NULL)
        return new_node;
    int date_compare = compare_dates(new_node->check_in, node->check_in);
    if (date_compare < 0)
    {
        node->left = insert_booking(node->left, new_node);
    }
    else
    {
        node->right = insert_booking(node->right, new_node);
    }
    return node;
}

// Search bookings by guest ID
void search_bookings_by_guest(BookingNode *root, char *guest_id)
{
    if (root != NULL)
    {
        search_bookings_by_guest(root->left, guest_id);
        if (strcmp(root->guest_id, guest_id) == 0)
        {
            char *status_str[] = {"Upcoming", "Active", "Completed", "Cancelled"};
            printf("Booking ID: %s, Room %d, Check-in: %02d/%02d/%d, Status: %s\n",
                   root->booking_id, root->room_no, root->check_in.day,
                   root->check_in.month, root->check_in.year, status_str[root->status]);
        }
        search_bookings_by_guest(root->right, guest_id);
    }
}

// Find booking
BookingNode *find_booking(BookingNode *root, char *booking_id)
{
    if (root == NULL)
        return NULL;
    if (strcmp(root->booking_id, booking_id) == 0)
        return root;
    BookingNode *left = find_booking(root->left, booking_id);
    if (left != NULL)
        return left;
    return find_booking(root->right, booking_id);
}

// Update booking status
void update_booking_status(BookingNode *root, Date current)
{
    if (root != NULL)
    {
        update_booking_status(root->left, current);
        if (root->status == 0 && compare_dates(current, root->check_in) >= 0)
        {
            root->status = 1;
        }
        if (root->status == 1 && compare_dates(current, root->check_out) >= 0)
        {
            root->status = 2;
        }
        update_booking_status(root->right, current);
    }
}

// ======================== GRAPH OPERATIONS ========================

// Initialize facility graph
void init_facility_graph()
{
    facilities[0].adjacent[1] = 1;
    facilities[0].adjacent[2] = 1;
    facilities[1].adjacent[0] = 1;
    facilities[1].adjacent[3] = 1;
    facilities[2].adjacent[0] = 1;
    facilities[2].adjacent[3] = 1;
    facilities[3].adjacent[1] = 1;
    facilities[3].adjacent[2] = 1;
    facilities[3].adjacent[4] = 1;
    facilities[4].adjacent[3] = 1;
}

// Find nearby facilities
void find_nearby_facilities(int facility_id)
{
    int i;
    if (facility_id < 1 || facility_id > FACILITIES)
    {
        printf("Invalid facility ID!\n");
        return;
    }
    printf("Facilities near %s:\n", facilities[facility_id - 1].name);
    for (i = 0; i < FACILITIES; i++)
    {
        if (facilities[facility_id - 1].adjacent[i] == 1)
        {
            printf("- %s ($%.2f)\n", facilities[i].name, facilities[i].booking_fee);
        }
    }
}

// ======================== CORE SYSTEM FUNCTIONS ========================

void init_system()
{
    srand(time(0));
    int i;
    // Initialize rooms
    for (i = 0; i < MAX_ROOMS; i++)
    {
        rooms[i].room_no = i + 1;
        rooms[i].type = (i % 3) + 1;
        rooms[i].status = 0;
        rooms[i].floor = (i / 10) + 1;
        rooms[i].capacity = (rooms[i].type == 1) ? 2 : (rooms[i].type == 2) ? 4
                                                                            : 6;
        if (rooms[i].type == 1)
        {
            rooms[i].price = 100 + (rand() % 50);
            strcpy(rooms[i].features, "TV, WiFi, AC");
        }
        else if (rooms[i].type == 2)
        {
            rooms[i].price = 200 + (rand() % 100);
            strcpy(rooms[i].features, "TV, WiFi, AC, Mini-bar, Balcony");
        }
        else
        {
            rooms[i].price = 500 + (rand() % 200);
            strcpy(rooms[i].features, "TV, WiFi, AC, Mini-bar, Jacuzzi, Living area");
        }
    }

    // Initialize parking
    for (i = MAX_PARKING; i >= 1; i--)
    {
        add_parking_slot(&parking_slots, i);
    }

    // Initialize facilities
    int j;
    char *fac_names[] = {"Gym", "Pool", "Spa", "Restaurant", "Conference Room"};
    float fac_fees[] = {10.0, 5.0, 50.0, 0.0, 100.0};
    for (i = 0; i < FACILITIES; i++)
    {
        facilities[i].facility_id = i + 1;
        strcpy(facilities[i].name, fac_names[i]);
        facilities[i].status = 0;
        facilities[i].booking_fee = fac_fees[i];
        for (j = 0; j < FACILITIES; j++)
        {
            facilities[i].adjacent[j] = 0;
        }
    }
    init_facility_graph();

    // Initialize users
    strcpy(users[0].id, "admin");
    strcpy(users[0].name, "Administrator");
    strcpy(users[0].pass, "admin123");
    strcpy(users[0].role, "admin");
    strcpy(users[0].department, "Management");
    users[0].access_level = 5;
    user_count = 1;

    // Initialize sample inventory
    char *inv_names[] = {"Towels", "Bed Sheets", "Shampoo", "Coffee", "Light Bulbs"};
    char *inv_cats[] = {"Housekeeping", "Housekeeping", "Housekeeping", "Food", "Maintenance"};
    float inv_prices[] = {5.0, 10.0, 2.0, 3.0, 4.0};
    int inv_quants[] = {100, 50, 200, 300, 150};
    int inv_reorder[] = {20, 10, 50, 50, 30};
    for (i = 0; i < 5; i++)
    {
        inventory[i].item_id = i + 1;
        strcpy(inventory[i].name, inv_names[i]);
        strcpy(inventory[i].category, inv_cats[i]);
        inventory[i].quantity = inv_quants[i];
        inventory[i].unit_price = inv_prices[i];
        inventory[i].reorder_level = inv_reorder[i];
        inventory_count++;
    }
}

void add_guest(char *user_id)
{
    if (guest_count >= MAX_GUESTS)
    {
        printf("Max guests reached!\n");
        return;
    }
    Guest new_guest;
    printf("Enter guest ID: ");
    scanf("%s", new_guest.id);
    if (find_guest_in_list(guest_list, new_guest.id) != NULL)
    {
        printf("Guest ID exists!\n");
        return;
    }
    printf("Enter name: ");
    scanf(" %[^\n]", new_guest.name);
    printf("Enter contact: ");
    scanf("%s", new_guest.contact);
    printf("Enter email: ");
    scanf("%s", new_guest.email);
    printf("Enter preferences (optional): ");
    scanf(" %[^\n]", new_guest.preferences);
    add_guest_to_list(&guest_list, new_guest);
    printf("Guest added successfully!\n");
    char log_msg[100];
    sprintf(log_msg, "Added guest %s", new_guest.id);
    log_activity(user_id, log_msg);
}

void view_guest_details()
{
    char id[MAX_ID];
    printf("Enter guest ID: ");
    scanf("%s", id);
    Guest *guest = find_guest_in_list(guest_list, id);
    if (guest == NULL)
    {
        printf("Guest not found!\n");
        return;
    }
    printf("\n=== GUEST DETAILS ===\n");
    printf("ID: %s\nName: %s\nContact: %s\nEmail: %s\n",
           guest->id, guest->name, guest->contact, guest->email);
    printf("Preferences: %s\nLoyalty Points: %d\n",
           guest->preferences, guest->loyalty_points);
    printf("\nBooking History:\n");
    search_bookings_by_guest(booking_tree, id);
}

void book_room(char *user_id)
{
    if (guest_count == 0)
    {
        printf("No guests registered!\n");
        return;
    }
    char guest_id[MAX_ID];
    printf("Enter guest ID: ");
    scanf("%s", guest_id);
    Guest *guest = find_guest_in_list(guest_list, guest_id);
    if (guest == NULL)
    {
        printf("Guest not found!\n");
        return;
    }
    int i;
    printf("\nAvailable Rooms:\n");
    printf("Room No\tType\tFloor\tCapacity\tPrice\tFeatures\n");
    for (i = 0; i < MAX_ROOMS; i++)
    {
        if (rooms[i].status == 0)
        {
            char *type = (rooms[i].type == 1) ? "Standard" : (rooms[i].type == 2) ? "Deluxe"
                                                                                  : "Suite";
            printf("%d\t%s\t%d\t%d\t\t$%.2f\t%s\n",
                   rooms[i].room_no, type, rooms[i].floor, rooms[i].capacity,
                   rooms[i].price, rooms[i].features);
        }
    }
    int room_no;
    printf("\nEnter room number: ");
    scanf("%d", &room_no);
    if (room_no < 1 || room_no > MAX_ROOMS || rooms[room_no - 1].status != 0)
    {
        printf("Invalid or unavailable room!\n");
        return;
    }
    Date ci, co;
    printf("Enter check-in date (dd mm yyyy): ");
    scanf("%d %d %d", &ci.day, &ci.month, &ci.year);
    printf("Enter check-out date (dd mm yyyy): ");
    scanf("%d %d %d", &co.day, &co.month, &co.year);
    if (!is_valid_date(ci) || !is_valid_date(co) || date_diff(ci, co) <= 0)
    {
        printf("Invalid dates!\n");
        return;
    }
    float base_price = rooms[room_no - 1].price * date_diff(ci, co);
    float discount = (guest->loyalty_points >= DISCOUNT_THRESHOLD) ? base_price * 0.1 : 0;
    float total = base_price - discount;
    if (discount > 0)
    {
        printf("Applied 10%% discount ($%.2f)!\n", discount);
        guest->loyalty_points -= DISCOUNT_THRESHOLD;
    }
    rooms[room_no - 1].status = 1;
    int points_earned = (int)(total / 100) * LOYALTY_POINTS_RATE;
    guest->loyalty_points += points_earned;
    BookingNode *new_booking = new_booking_node(guest_id, room_no, -1, -1, ci, co, total);
    booking_tree = insert_booking(booking_tree, new_booking);
    total_revenue += total;
    printf("\n=== BOOKING CONFIRMED ===\n");
    printf("Booking ID: %s\nRoom: %d\nCheck-in: %02d/%02d/%d\nCheck-out: %02d/%02d/%d\nTotal: $%.2f\n",
           new_booking->booking_id, room_no, ci.day, ci.month, ci.year, co.day, co.month, co.year, total);
    printf("Earned %d loyalty points. Total: %d\n", points_earned, guest->loyalty_points);
    char log_msg[100];
    sprintf(log_msg, "Booked room %d for guest %s", room_no, guest_id);
    log_activity(user_id, log_msg);
}

void assign_parking(char *user_id)
{
    char guest_id[MAX_ID];
    printf("Enter guest ID: ");
    scanf("%s", guest_id);
    ParkingSlot *slot = parking_slots;
    while (slot != NULL)
    {
        if (strcmp(slot->guest_id, guest_id) == 0)
        {
            printf("Guest already has slot %d!\n", slot->slot_no);
            return;
        }
        slot = slot->next;
    }
    slot = parking_slots;
    ParkingSlot *prev = NULL;
    while (slot != NULL)
    {
        if (slot->status == 0)
            break;
        prev = slot;
        slot = slot->next;
    }
    char vehicle[20];
    printf("Enter vehicle type: ");
    scanf(" %[^\n]", vehicle);
    if (slot != NULL)
    {
        slot->status = 1;
        strcpy(slot->vehicle, vehicle);
        strcpy(slot->guest_id, guest_id);
        BookingNode *booking = booking_tree;
        while (booking != NULL)
        {
            if (strcmp(booking->guest_id, guest_id) == 0 && booking->status < 2)
            {
                booking->parking_slot = slot->slot_no;
                break;
            }
            booking = booking->left;
        }
        printf("Assigned slot %d for %s\n", slot->slot_no, vehicle);
        char log_msg[100];
        sprintf(log_msg, "Assigned parking slot %d to guest %s", slot->slot_no, guest_id);
        log_activity(user_id, log_msg);
    }
    else
    {
        add_to_waitlist(&parking_waitlist, vehicle, guest_id);
        printf("No parking available. Added to waitlist.\n");
        char log_msg[100];
        sprintf(log_msg, "Added guest %s to parking waitlist", guest_id);
        log_activity(user_id, log_msg);
    }
}

void process_waitlist(char *user_id)
{
    ParkingSlot *waiting = parking_waitlist;
    if (waiting == NULL)
    {
        printf("No vehicles in waitlist.\n");
        return;
    }
    ParkingSlot *slot = parking_slots;
    while (slot != NULL && waiting != NULL)
    {
        if (slot->status == 0)
        {
            slot->status = 1;
            strcpy(slot->vehicle, waiting->vehicle);
            strcpy(slot->guest_id, waiting->guest_id);
            printf("Assigned slot %d to %s\n", slot->slot_no, waiting->vehicle);
            BookingNode *booking = booking_tree;
            while (booking != NULL)
            {
                if (strcmp(booking->guest_id, waiting->guest_id) == 0 && booking->status < 2)
                {
                    booking->parking_slot = slot->slot_no;
                    break;
                }
                booking = booking->left;
            }
            ParkingSlot *temp = waiting;
            waiting = waiting->next;
            free(temp);
            parking_waitlist = waiting;
            char log_msg[100];
            sprintf(log_msg, "Assigned parking slot %d from waitlist to guest %s",
                    slot->slot_no, slot->guest_id);
            log_activity(user_id, log_msg);
        }
        slot = slot->next;
    }
}

void book_facility(char *user_id)
{
    int i;
    printf("\nAvailable Facilities:\n");
    printf("ID\tName\t\tFee\tStatus\n");
    for (i = 0; i < FACILITIES; i++)
    {
        printf("%d\t%s\t$%.2f\t%s\n", facilities[i].facility_id, facilities[i].name,
               facilities[i].booking_fee, facilities[i].status ? "Booked" : "Available");
    }
    int facility_id;
    printf("\nEnter facility ID: ");
    scanf("%d", &facility_id);
    if (facility_id < 1 || facility_id > FACILITIES || facilities[facility_id - 1].status != 0)
    {
        printf("Invalid or booked facility!\n");
        return;
    }
    char guest_id[MAX_ID];
    printf("Enter guest ID: ");
    scanf("%s", guest_id);
    Guest *guest = find_guest_in_list(guest_list, guest_id);
    if (guest == NULL)
    {
        printf("Guest not found!\n");
        return;
    }
    Date date;
    printf("Enter booking date (dd mm yyyy): ");
    scanf("%d %d %d", &date.day, &date.month, &date.year);
    if (!is_valid_date(date))
    {
        printf("Invalid date!\n");
        return;
    }
    float fee = facilities[facility_id - 1].booking_fee;
    if (guest->loyalty_points >= DISCOUNT_THRESHOLD && fee > 0)
    {
        fee *= 0.5;
        printf("Applied 50%% discount! New fee: $%.2f\n", fee);
        guest->loyalty_points -= DISCOUNT_THRESHOLD;
    }
    facilities[facility_id - 1].status = 1;
    BookingNode *new_booking = new_booking_node(guest_id, -1, -1, facility_id, date, date, fee);
    booking_tree = insert_booking(booking_tree, new_booking);
    int points_earned = (facility_id == 3) ? 0 : 5;
    guest->loyalty_points += points_earned;
    total_revenue += fee;
    printf("\n=== FACILITY BOOKING CONFIRMED ===\n");
    printf("Booking ID: %s\nFacility: %s\nDate: %02d/%02d/%d\nFee: $%.2f\n",
           new_booking->booking_id, facilities[facility_id - 1].name,
           date.day, date.month, date.year, fee);
    printf("Earned %d loyalty points. Total: %d\n", points_earned, guest->loyalty_points);
    find_nearby_facilities(facility_id);
    char log_msg[100];
    sprintf(log_msg, "Booked facility %s for guest %s", facilities[facility_id - 1].name, guest_id);
    log_activity(user_id, log_msg);
}

void order_room_service(char *user_id)
{
    if (service_count >= MAX_SERVICES)
    {
        printf("Max service orders reached!\n");
        return;
    }
    int room_no;
    printf("Enter room number: ");
    scanf("%d", &room_no);
    if (room_no < 1 || room_no > MAX_ROOMS)
    {
        printf("Invalid room number!\n");
        return;
    }
    int occupied = 0;
    BookingNode *booking = booking_tree;
    while (booking != NULL)
    {
        if (booking->room_no == room_no && booking->status == 1)
        {
            occupied = 1;
            break;
        }
        booking = booking->left;
    }
    if (!occupied)
    {
        printf("Room not occupied!\n");
        return;
    }
    printf("Available items:\n");
    printf("1. Breakfast Set ($15.00)\n2. Lunch Set ($20.00)\n3. Dinner Set ($25.00)\n");
    printf("4. Snack Pack ($10.00)\n5. Beverage ($5.00)\n");
    int choice;
    printf("Enter item number: ");
    scanf("%d", &choice);
    if (choice < 1 || choice > 5)
    {
        printf("Invalid choice!\n");
        return;
    }
    char *items[] = {"Breakfast Set", "Lunch Set", "Dinner Set", "Snack Pack", "Beverage"};
    float prices[] = {15.0, 20.0, 25.0, 10.0, 5.0};
    char time_str[10];
    get_current_time(time_str);
    services[service_count].service_id = service_count + 1;
    services[service_count].room_no = room_no;
    strcpy(services[service_count].items, items[choice - 1]);
    strcpy(services[service_count].time, time_str);
    strcpy(services[service_count].status, "Pending");
    services[service_count].charge = prices[choice - 1];
    printf("\n=== SERVICE ORDER CREATED ===\n");
    printf("Order ID: %d\nRoom: %d\nItems: %s\nTime: %s\nCharge: $%.2f\n",
           services[service_count].service_id, room_no, items[choice - 1], time_str, prices[choice - 1]);
    service_count++;
    char log_msg[100];
    sprintf(log_msg, "Ordered room service %s for room %d", items[choice - 1], room_no);
    log_activity(user_id, log_msg);
}

void update_service_status(char *user_id)
{
    int i;
    if (service_count == 0)
    {
        printf("No service orders!\n");
        return;
    }
    printf("Current service orders:\n");
    printf("ID\tRoom\tItems\t\tTime\tStatus\tCharge\n");
    for (i = 0; i < service_count; i++)
    {
        printf("%d\t%d\t%s\t%s\t%s\t$%.2f\n",
               services[i].service_id, services[i].room_no, services[i].items,
               services[i].time, services[i].status, services[i].charge);
    }
    int id, choice;
    printf("Enter service ID: ");
    scanf("%d", &id);
    int found = 0;
    for (i = 0; i < service_count; i++)
    {
        if (services[i].service_id == id)
        {
            found = 1;
            printf("\nCurrent status: %s\n1. Mark as In Progress\n2. Mark as Completed\n",
                   services[i].status);
            printf("Enter choice: ");
            scanf("%d", &choice);
            if (choice == 1)
            {
                strcpy(services[i].status, "In Progress");
                printf("Status updated to In Progress\n");
            }
            else if (choice == 2)
            {
                strcpy(services[i].status, "Completed");
                BookingNode *booking = booking_tree;
                while (booking != NULL)
                {
                    if (booking->room_no == services[i].room_no && booking->status == 1)
                    {
                        Guest *guest = find_guest_in_list(guest_list, booking->guest_id);
                        if (guest != NULL)
                            guest->loyalty_points += 1;
                        break;
                    }
                    booking = booking->left;
                }
                printf("Status updated to Completed. Guest earned 1 point.\n");
            }
            else
            {
                printf("Invalid choice!\n");
            }
            char log_msg[100];
            sprintf(log_msg, "Updated service order %d status to %s", id,
                    choice == 1 ? "In Progress" : "Completed");
            log_activity(user_id, log_msg);
            break;
        }
    }
    if (!found)
        printf("Service ID not found!\n");
}

void report_maintenance(char *user_id)
{
    if (maintenance_count >= MAX_MAINTENANCE)
    {
        printf("Max maintenance requests reached!\n");
        return;
    }
    int room_no;
    printf("Enter room number: ");
    scanf("%d", &room_no);
    if (room_no < 1 || room_no > MAX_ROOMS)
    {
        printf("Invalid room number!\n");
        return;
    }
    printf("Issue types:\n1. Electrical\n2. Plumbing\n3. HVAC\n4. Furniture\n5. Other\n");
    int issue_type;
    printf("Enter issue type: ");
    scanf("%d", &issue_type);
    if (issue_type < 1 || issue_type > 5)
    {
        printf("Invalid issue type!\n");
        return;
    }
    printf("Priority:\n1. Low\n2. Medium\n3. High\n");
    int priority;
    printf("Enter priority: ");
    scanf("%d", &priority);
    if (priority < 1 || priority > 3)
    {
        printf("Invalid priority!\n");
        return;
    }
    char description[100];
    printf("Enter description: ");
    scanf(" %[^\n]", description);
    char *issues[] = {"Electrical", "Plumbing", "HVAC", "Furniture", "Other"};
    char *priorities[] = {"Low", "Medium", "High"};
    maintenance[maintenance_count].request_id = maintenance_count + 1;
    maintenance[maintenance_count].room_no = room_no;
    strcpy(maintenance[maintenance_count].issue, issues[issue_type - 1]);
    strcat(maintenance[maintenance_count].issue, ": ");
    strcat(maintenance[maintenance_count].issue, description);
    strcpy(maintenance[maintenance_count].priority, priorities[priority - 1]);
    strcpy(maintenance[maintenance_count].status, "Open");
    maintenance[maintenance_count].report_date = get_current_date();
    rooms[room_no - 1].status = 2;
    printf("\n=== MAINTENANCE REQUEST CREATED ===\n");
    printf("Request ID: %d\nRoom: %d\nIssue: %s\nPriority: %s\nDate: %02d/%02d/%d\n",
           maintenance[maintenance_count].request_id, room_no,
           maintenance[maintenance_count].issue, priorities[priority - 1],
           maintenance[maintenance_count].report_date.day,
           maintenance[maintenance_count].report_date.month,
           maintenance[maintenance_count].report_date.year);
    maintenance_count++;
    char log_msg[100];
    sprintf(log_msg, "Reported maintenance issue for room %d", room_no);
    log_activity(user_id, log_msg);
}

void update_maintenance_status(char *user_id)
{
    int i;
    if (maintenance_count == 0)
    {
        printf("No maintenance requests!\n");
        return;
    }
    printf("Maintenance Requests:\n");
    printf("ID\tRoom\tIssue\tPriority\tStatus\tDate\n");
    for (i = 0; i < maintenance_count; i++)
    {
        printf("%d\t%d\t%s\t%s\t%s\t%02d/%02d/%d\n",
               maintenance[i].request_id, maintenance[i].room_no, maintenance[i].issue,
               maintenance[i].priority, maintenance[i].status,
               maintenance[i].report_date.day, maintenance[i].report_date.month,
               maintenance[i].report_date.year);
    }
    int id, choice;
    printf("Enter request ID: ");
    scanf("%d", &id);
    int found = 0;
    for (i = 0; i < maintenance_count; i++)
    {
        if (maintenance[i].request_id == id)
        {
            found = 1;
            printf("\nCurrent status: %s\n1. Mark as In Progress\n2. Mark as Resolved\n",
                   maintenance[i].status);
            printf("Enter choice: ");
            scanf("%d", &choice);
            if (choice == 1)
            {
                strcpy(maintenance[i].status, "In Progress");
                printf("Status updated to In Progress\n");
            }
            else if (choice == 2)
            {
                strcpy(maintenance[i].status, "Resolved");
                rooms[maintenance[i].room_no - 1].status = 0;
                printf("Status updated to Resolved. Room available.\n");
            }
            else
            {
                printf("Invalid choice!\n");
            }
            char log_msg[100];
            sprintf(log_msg, "Updated maintenance request %d status to %s", id,
                    choice == 1 ? "In Progress" : "Resolved");
            log_activity(user_id, log_msg);
            break;
        }
    }
    if (!found)
        printf("Request ID not found!\n");
}

void submit_feedback(char *user_id)
{
    if (feedback_count >= MAX_FEEDBACK)
    {
        printf("Max feedback reached!\n");
        return;
    }
    char guest_id[MAX_ID];
    printf("Enter guest ID: ");
    scanf("%s", guest_id);
    if (find_guest_in_list(guest_list, guest_id) == NULL)
    {
        printf("Guest not found!\n");
        return;
    }
    char comment[200];
    int rating;
    printf("Enter comment: ");
    scanf(" %[^\n]", comment);
    printf("Enter rating (1-5): ");
    scanf("%d", &rating);
    if (rating < 1 || rating > 5)
    {
        printf("Invalid rating!\n");
        return;
    }
    feedbacks[feedback_count].feedback_id = feedback_count + 1;
    strcpy(feedbacks[feedback_count].guest_id, guest_id);
    strcpy(feedbacks[feedback_count].comment, comment);
    feedbacks[feedback_count].rating = rating;
    feedbacks[feedback_count].date = get_current_date();
    feedback_count++;
    printf("Feedback submitted successfully!\n");
    char log_msg[100];
    sprintf(log_msg, "Guest %s submitted feedback with rating %d", guest_id, rating);
    log_activity(user_id, log_msg);
}

void manage_inventory(char *user_id)
{
    int choice;
    int i;
    do
    {
        printf("\n=== INVENTORY MANAGEMENT ===\n");
        printf("1. View Inventory\n2. Add Item\n3. Update Quantity\n4. Check Low Stock\n5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Inventory:\n");
            printf("ID\tName\tCategory\tQuantity\tPrice\tReorder Level\n");
            for (i = 0; i < inventory_count; i++)
            {
                printf("%d\t%s\t%s\t%d\t$%.2f\t%d\n",
                       inventory[i].item_id, inventory[i].name, inventory[i].category,
                       inventory[i].quantity, inventory[i].unit_price, inventory[i].reorder_level);
            }
            break;
        case 2:
            if (inventory_count >= MAX_INVENTORY)
            {
                printf("Max inventory reached!\n");
                break;
            }
            printf("Enter item name: ");
            scanf(" %[^\n]", inventory[inventory_count].name);
            printf("Enter category: ");
            scanf(" %[^\n]", inventory[inventory_count].category);
            printf("Enter quantity: ");
            scanf("%d", &inventory[inventory_count].quantity);
            printf("Enter unit price: ");
            scanf("%f", &inventory[inventory_count].unit_price);
            printf("Enter reorder level: ");
            scanf("%d", &inventory[inventory_count].reorder_level);
            inventory[inventory_count].item_id = inventory_count + 1;
            inventory_count++;
            printf("Item added!\n");
            log_activity(user_id, "Added new inventory item");
            break;
        case 3:
            printf("Enter item ID: ");
            int id;
            scanf("%d", &id);
            int found = 0;
            for (i = 0; i < inventory_count; i++)
            {
                if (inventory[i].item_id == id)
                {
                    found = 1;
                    int qty;
                    printf("Enter new quantity: ");
                    scanf("%d", &qty);
                    inventory[i].quantity = qty;
                    printf("Quantity updated!\n");
                    char log_msg[100];
                    sprintf(log_msg, "Updated quantity for item %d", id);
                    log_activity(user_id, log_msg);
                    break;
                }
            }
            if (!found)
                printf("Item not found!\n");
            break;
        case 4:
            printf("Low Stock Items:\n");
            for (i = 0; i < inventory_count; i++)
            {
                if (inventory[i].quantity <= inventory[i].reorder_level)
                {
                    printf("%d\t%s\t%d (Reorder at %d)\n",
                           inventory[i].item_id, inventory[i].name,
                           inventory[i].quantity, inventory[i].reorder_level);
                }
            }
            break;
        case 5:
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

void manage_events(char *user_id)
{
    int choice;
    int i;
    do
    {
        printf("\n=== EVENT MANAGEMENT ===\n");
        printf("1. View Events\n2. Add Event\n3. Update Event Status\n4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Events:\n");
            printf("ID\tName\tDate\tTime\tFacility\tStatus\n");
            for (i = 0; i < event_count; i++)
            {
                char *status[] = {"Planned", "Ongoing", "Completed"};
                printf("%d\t%s\t%02d/%02d/%d\t%s\t%s\t%s\n",
                       events[i].event_id, events[i].name,
                       events[i].date.day, events[i].date.month, events[i].date.year,
                       events[i].time, facilities[events[i].facility_id - 1].name,
                       status[events[i].status]);
            }
            break;
        case 2:
            if (event_count >= MAX_EVENTS)
            {
                printf("Max events reached!\n");
                break;
            }
            printf("Enter event name: ");
            scanf(" %[^\n]", events[event_count].name);
            printf("Enter date (dd mm yyyy): ");
            scanf("%d %d %d", &events[event_count].date.day,
                  &events[event_count].date.month, &events[event_count].date.year);
            printf("Enter time (HH:MM): ");
            scanf("%s", events[event_count].time);
            printf("Enter facility ID: ");
            scanf("%d", &events[event_count].facility_id);
            printf("Enter organizer: ");
            scanf(" %[^\n]", events[event_count].organizer);
            printf("Enter capacity: ");
            scanf("%d", &events[event_count].capacity);
            printf("Enter cost: ");
            scanf("%f", &events[event_count].cost);
            events[event_count].event_id = event_count + 1;
            events[event_count].status = 0;
            event_count++;
            printf("Event added!\n");
            char log_msg[100];
            sprintf(log_msg, "Added event %s", events[event_count - 1].name);
            log_activity(user_id, log_msg);
            break;
        case 3:
            printf("Enter event ID: ");
            int id;
            scanf("%d", &id);
            int found = 0;
            for (i = 0; i < event_count; i++)
            {
                if (events[i].event_id == id)
                {
                    found = 1;
                    printf("Current status: %s\n1. Mark as Ongoing\n2. Mark as Completed\n",
                           events[i].status == 0 ? "Planned" : events[i].status == 1 ? "Ongoing"
                                                                                     : "Completed");
                    int status;
                    printf("Enter new status: ");
                    scanf("%d", &status);
                    if (status == 1 || status == 2)
                    {
                        events[i].status = status;
                        printf("Status updated!\n");
                        char log_msg[100];
                        sprintf(log_msg, "Updated event %d status", id);
                        log_activity(user_id, log_msg);
                    }
                    else
                    {
                        printf("Invalid status!\n");
                    }
                    break;
                }
            }
            if (!found)
                printf("Event not found!\n");
            break;
        case 4:
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 4);
}

void manage_staff_schedule(char *user_id)
{
    int choice;
    int i;
    do
    {
        printf("\n=== STAFF SCHEDULING ===\n");
        printf("1. View Schedules\n2. Add Schedule\n3. Update Schedule\n4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Schedules:\n");
            printf("ID\tStaff ID\tDate\tShift\tTask\n");
            for (i = 0; i < schedule_count; i++)
            {
                printf("%d\t%s\t%02d/%02d/%d\t%s\t%s\n",
                       schedules[i].schedule_id, schedules[i].staff_id,
                       schedules[i].date.day, schedules[i].date.month, schedules[i].date.year,
                       schedules[i].shift, schedules[i].task);
            }
            break;
        case 2:
            if (schedule_count >= MAX_SCHEDULES)
            {
                printf("Max schedules reached!\n");
                break;
            }
            printf("Enter staff ID: ");
            scanf("%s", schedules[schedule_count].staff_id);
            int staff_found = 0;
            for (i = 0; i < user_count; i++)
            {
                if (strcmp(users[i].id, schedules[schedule_count].staff_id) == 0)
                {
                    staff_found = 1;
                    break;
                }
            }
            if (!staff_found)
            {
                printf("Staff ID not found!\n");
                break;
            }
            printf("Enter date (dd mm yyyy): ");
            scanf("%d %d %d", &schedules[schedule_count].date.day,
                  &schedules[schedule_count].date.month, &schedules[schedule_count].date.year);
            printf("Enter shift (Morning/Evening/Night): ");
            scanf("%s", schedules[schedule_count].shift);
            printf("Enter task: ");
            scanf(" %[^\n]", schedules[schedule_count].task);
            schedules[schedule_count].schedule_id = schedule_count + 1;
            schedule_count++;
            printf("Schedule added!\n");
            char log_msg[100];
            sprintf(log_msg, "Added schedule for staff %s", schedules[schedule_count - 1].staff_id);
            log_activity(user_id, log_msg);
            break;
        case 3:
            printf("Enter schedule ID: ");
            int id;
            scanf("%d", &id);
            int found = 0;
            for (i = 0; i < schedule_count; i++)
            {
                if (schedules[i].schedule_id == id)
                {
                    found = 1;
                    printf("Current Schedule: %s, %s, %02d/%02d/%d\n",
                           schedules[i].staff_id, schedules[i].shift,
                           schedules[i].date.day, schedules[i].date.month, schedules[i].date.year);
                    printf("Enter new shift: ");
                    scanf("%s", schedules[i].shift);
                    printf("Enter new task: ");
                    scanf(" %[^\n]", schedules[i].task);
                    printf("Schedule updated!\n");
                    char log_msg[100];
                    sprintf(log_msg, "Updated schedule %d", id);
                    log_activity(user_id, log_msg);
                    break;
                }
            }
            if (!found)
                printf("Schedule not found!\n");
            break;
        case 4:
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 4);
}

void generate_bill(char *user_id)
{
    if (bill_count >= MAX_BILLS)
    {
        printf("Max bills reached!\n");
        return;
    }
    char guest_id[MAX_ID];
    printf("Enter guest ID: ");
    scanf("%s", guest_id);
    Guest *guest = find_guest_in_list(guest_list, guest_id);
    if (guest == NULL)
    {
        printf("Guest not found!\n");
        return;
    }
    char booking_id[10];
    printf("Enter booking ID: ");
    scanf("%s", booking_id);
    BookingNode *booking = find_booking(booking_tree, booking_id);
    if (booking == NULL || strcmp(booking->guest_id, guest_id) != 0)
    {
        printf("Invalid booking ID!\n");
        return;
    }
    float room_charges = 0, service_charges = 0, facility_charges = 0;
    if (booking->room_no != -1)
    {
        room_charges = booking->total;
    }
    if (booking->facility_id != -1)
    {
        facility_charges = booking->total;
    }
    int i;
    for (i = 0; i < service_count; i++)
    {
        if (services[i].room_no == booking->room_no && strcmp(services[i].status, "Completed") == 0)
        {
            service_charges += services[i].charge;
        }
    }
    float taxes = (room_charges + service_charges + facility_charges) * 0.1; // 10% tax
    float total = room_charges + service_charges + facility_charges + taxes;
    bills[bill_count].bill_id = bill_count + 1;
    strcpy(bills[bill_count].guest_id, guest_id);
    strcpy(bills[bill_count].booking_id, booking_id);
    bills[bill_count].room_charges = room_charges;
    bills[bill_count].service_charges = service_charges;
    bills[bill_count].facility_charges = facility_charges;
    bills[bill_count].taxes = taxes;
    bills[bill_count].total = total;
    bills[bill_count].status = 0;
    bills[bill_count].issue_date = get_current_date();
    printf("\n=== BILL GENERATED ===\n");
    printf("Bill ID: %d\nGuest: %s\nBooking ID: %s\n", bills[bill_count].bill_id, guest_id, booking_id);
    printf("Room Charges: $%.2f\nService Charges: $%.2f\nFacility Charges: $%.2f\n",
           room_charges, service_charges, facility_charges);
    printf("Taxes (10%%): $%.2f\nTotal: $%.2f\nStatus: Unpaid\n", taxes, total);
    bill_count++;
    char log_msg[100];
    sprintf(log_msg, "Generated bill %d for guest %s", bills[bill_count - 1].bill_id, guest_id);
    log_activity(user_id, log_msg);
}

void process_payment(char *user_id)
{
    printf("Enter bill ID: ");
    int i;
    int id;
    scanf("%d", &id);
    int found = 0;
    for (i = 0; i < bill_count; i++)
    {
        if (bills[i].bill_id == id)
        {
            found = 1;
            if (bills[i].status == 1)
            {
                printf("Bill already paid!\n");
                return;
            }
            printf("Bill Details:\nTotal: $%.2f\n", bills[i].total);
            printf("1. Pay by Cash\n2. Pay by Card\n3. Cancel\n");
            int method;
            printf("Enter payment method: ");
            scanf("%d", &method);
            if (method == 1 || method == 2)
            {
                bills[i].status = 1;
                total_revenue += bills[i].total;
                Guest *guest = find_guest_in_list(guest_list, bills[i].guest_id);
                if (guest != NULL)
                {
                    guest->loyalty_points += (int)(bills[i].total / 100) * LOYALTY_POINTS_RATE;
                }
                printf("Payment successful! Bill marked as paid.\n");
                char log_msg[100];
                sprintf(log_msg, "Processed payment for bill %d", id);
                log_activity(user_id, log_msg);
            }
            else
            {
                printf("Payment cancelled.\n");
            }
            break;
        }
    }
    if (!found)
        printf("Bill not found!\n");
}

void analytics_dashboard(char *user_id)
{
    int i;
    printf("\n=== ANALYTICS DASHBOARD ===\n");
    int occupied_rooms = 0;
    for (i = 0; i < MAX_ROOMS; i++)
    {
        if (rooms[i].status == 1)
            occupied_rooms++;
    }
    printf("Room Occupancy: %d/%d (%.2f%%)\n", occupied_rooms, MAX_ROOMS,
           (float)occupied_rooms / MAX_ROOMS * 100);
    int occupied_parking = 0;
    ParkingSlot *slot = parking_slots;
    while (slot != NULL)
    {
        if (slot->status == 1)
            occupied_parking++;
        slot = slot->next;
    }
    printf("Parking Occupancy: %d/%d (%.2f%%)\n", occupied_parking, MAX_PARKING,
           (float)occupied_parking / MAX_PARKING * 100);
    float avg_rating = 0;
    for (i = 0; i < feedback_count; i++)
    {
        avg_rating += feedbacks[i].rating;
    }
    avg_rating = feedback_count > 0 ? avg_rating / feedback_count : 0;
    printf("Average Guest Rating: %.2f/5\n", avg_rating);
    printf("Total Revenue: $%.2f\n", total_revenue);
    printf("Pending Maintenance Requests: %d\n", maintenance_count);
    printf("Active Bookings: %d\n", booking_count);
    char log_msg[100];
    sprintf(log_msg, "Viewed analytics dashboard");
    log_activity(user_id, log_msg);
}

void view_logs(char *user_id)
{
    int i;
    if (log_count == 0)
    {
        printf("No logs available!\n");
        return;
    }
    printf("\n=== SYSTEM LOGS ===\n");
    printf("ID\tUser\tAction\tDate\tTime\n");
    for (i = 0; i < log_count; i++)
    {
        printf("%d\t%s\t%s\t%02d/%02d/%d\t%s\n",
               logs[i].log_id, logs[i].user_id, logs[i].action,
               logs[i].date.day, logs[i].date.month, logs[i].date.year, logs[i].time);
    }
    char log_msg[100];
    sprintf(log_msg, "Viewed system logs");
    log_activity(user_id, log_msg);
}

void login()
{
    int i;
    char id[MAX_ID], pass[MAX_PASS];
    printf("Enter ID: ");
    scanf("%s", id);
    printf("Enter password: ");
    scanf("%s", pass);
    int access_level = 0;
    char user_id[MAX_ID];
    for (i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].id, id) == 0 && strcmp(users[i].pass, pass) == 0)
        {
            access_level = users[i].access_level;
            strcpy(user_id, users[i].id);
            break;
        }
    }
    if (access_level == 0)
    {
        printf("Invalid credentials!\n");
        return;
    }
    printf("Login successful! Access level: %d\n", access_level);
    int choice;
    do
    {
        printf("\n=== HOTEL MANAGEMENT SYSTEM ===\n");
        printf("1. Add Guest\n2. View Guest Details\n3. Book Room\n4. Assign Parking\n");
        printf("5. Process Parking Waitlist\n6. Book Facility\n7. Order Room Service\n");
        printf("8. Update Service Status\n9. Report Maintenance\n10. Update Maintenance Status\n");
        printf("11. Submit Feedback\n12. Manage Inventory\n13. Manage Events\n");
        printf("14. Manage Staff Schedule\n15. Generate Bill\n16. Process Payment\n");
        printf("17. Analytics Dashboard\n");
        if (access_level >= 5)
            printf("18. View System Logs\n19. Exit\n");
        else
            printf("18. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            add_guest(user_id);
            break;
        case 2:
            view_guest_details();
            break;
        case 3:
            book_room(user_id);
            break;
        case 4:
            assign_parking(user_id);
            break;
        case 5:
            process_waitlist(user_id);
            break;
        case 6:
            book_facility(user_id);
            break;
        case 7:
            order_room_service(user_id);
            break;
        case 8:
            update_service_status(user_id);
            break;
        case 9:
            report_maintenance(user_id);
            break;
        case 10:
            update_maintenance_status(user_id);
            break;
        case 11:
            submit_feedback(user_id);
            break;
        case 12:
            manage_inventory(user_id);
            break;
        case 13:
            manage_events(user_id);
            break;
        case 14:
            manage_staff_schedule(user_id);
            break;
        case 15:
            generate_bill(user_id);
            break;
        case 16:
            process_payment(user_id);
            break;
        case 17:
            analytics_dashboard(user_id);
            break;
        case 18:
            if (access_level >= 5)
                view_logs(user_id);
            else
                choice = 19;
            break;
        case 19:
            if (access_level >= 5)
                break;
            else
                printf("Invalid choice!\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != (access_level >= 5 ? 19 : 18));
}

int main()
{
    init_system();
    printf("Welcome to Hotel Management System\n");
    login();
    return 0;
}