#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#define BIG_INT 9999

using namespace std;


#define INFINITY_SYMBOL "INF"
struct Router {
    string name;
    vector<vector<string>> distance_table;
};

static string user_input;
static vector<string> user_inputs;
static int router_count;

/**
 * Variable <converged> become "true" when no changes in all routing tables.
*/
static bool converged = false;

static vector<Router> routers;
static int timing_round = 0;
static int n = 0;
static vector<vector<Router>> summary;
static vector<int> t_index;

// Function to print the distance tables of all routers
void display() {
    for (const auto& router : routers) {
        for (const auto& row : router.distance_table) {
            for (const auto& cost : row) {
                cout << cost << " ";
            }
            cout << endl;
        }
    }
}

// Function to split a string input and a delimiter character
vector<string> split_string(string input, char delimiter) {
    vector<string> result;
    stringstream ss(input);
    string token;
    while (getline(ss, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

// Function to check whether the distance tables of the routers
// in the two vectors are equal
bool operator==(const vector<Router>& a, const vector<Router>& b) {
    for (int i = 0; i < router_count; i++) {
        for (int row = 1; row < router_count; row++) {
            for (int col = 1; col < router_count; col++) {
                if (a[i].distance_table[row][col] != b[i].distance_table[row][col]) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Function to get location of the destination router name in the distance table
int get_location(const vector<vector<string>>& distance_table, const string& destination_router_name) {
    int local = 0;
    for (const auto& row : distance_table) {
        for (const auto& col : row) {
            if (col == destination_router_name) {
                return local;
            }
            local++;
        }
    }
    return -1;
}

// Function to perform the calculation of the cost from the source router,
// the intermediate router to the destination router
string calculate(int router_index, int row, int col) {
    int index = 0;
    int result = 0;
    int source_to_intermediate_cost, intermediate_to_destination_cost = 0;

    const string& intermediate_router_name = routers[router_index].distance_table[0][col];
    const string& destination_router_name = routers[router_index].distance_table[row][0];

    string source_to_intermediate_cost_s = routers[router_index].distance_table[col][col];
    if (source_to_intermediate_cost_s == INFINITY_SYMBOL){
        return INFINITY_SYMBOL;
    }

    source_to_intermediate_cost = stoi(source_to_intermediate_cost_s);

    for (int i = 0; i < router_count; i++) {
        if (routers[i].name == intermediate_router_name) {
            index = i;
            break;
        }
    }

    int intermediate_to_dest_index = get_location(routers[index].distance_table, destination_router_name);
    
    string intermediate_to_destination_cost_s = routers[index].distance_table[intermediate_to_dest_index][intermediate_to_dest_index];
    if (intermediate_to_destination_cost_s == INFINITY_SYMBOL){
        return INFINITY_SYMBOL;
    }

    intermediate_to_destination_cost = stoi(intermediate_to_destination_cost_s);

    result = source_to_intermediate_cost + intermediate_to_destination_cost;

    return to_string(result);
}

// Function to calculate the minimum cost path from a destination router to any other router 
// in the network
string min_path(const vector<vector<string>>& distance_table, int dest_index, int router_count, string & hopping_node) {
    const int tmp = BIG_INT;
    int min_cost = tmp;

    for (int i = 1; i < router_count; i++) {
        string cur_hopping_node = distance_table[0][i];
        const string& cost_via_cur_hopping_node = distance_table[dest_index][i];
        if (cost_via_cur_hopping_node != INFINITY_SYMBOL && cost_via_cur_hopping_node != "-1") {
            int tmp = stoi(cost_via_cur_hopping_node);
            if (tmp < min_cost) {
                min_cost = tmp;
                hopping_node = cur_hopping_node;
            }
        }
    }

    if (min_cost != BIG_INT) {
        return to_string(min_cost);
    }
    return INFINITY_SYMBOL;
}

string min_path(const vector<vector<string>>& distance_table, int dest_index, int router_count){
    string hopping_node = "";
    return min_path(distance_table, dest_index, router_count, hopping_node);
}

// Function to return the name of the router, corresponding to the minimum cost path
string min_path_name(const vector<vector<string>>& distance_table, int dest_index, int router_count) {
    const int tmp = BIG_INT;
    int min_cost = tmp;
    string result = "original";

    for (int i = 1; i < router_count; i++) {
        const string& cost = distance_table[dest_index][i];
        if (cost != INFINITY_SYMBOL && cost != "-1") {
            int tmp = stoi(cost);
            if (tmp < min_cost) {
                min_cost = tmp;
                result = distance_table[0][i];
            }
        }
    }

    if (result != "original") {
        return result;
    }
    return "original";
}

// Function to calculate the cost of the optimized path 
string calculate_optimised(int router_index, int row, int col) {
    int intermediate_index = 0;
    int result = 0;
    int source_to_intermediate_cost, intermediate_to_destination_cost = 0;

    Router this_router = routers[router_index];
    const string& intermediate_router_name = this_router.distance_table[0][col];
    const string& destination_router_name = this_router.distance_table[row][0];

    if (this_router.distance_table[col][col] == "-1" || this_router.distance_table[col][col] == INFINITY_SYMBOL) {
        // this_router.distance_table[0][col] is not the neighbour node of node at this_router.
        return INFINITY_SYMBOL;
    } else {
        // process the neighbour node this_router.distance_table[0][col] 
        // of node at this_router.

        source_to_intermediate_cost = stoi(this_router.distance_table[col][col]);

        for (int j = 0; j < router_count; j++) {
            if (routers[j].name == intermediate_router_name) {
                intermediate_index = j;
                break;
            }
        }

        int dest_index = get_location(routers[intermediate_index].distance_table, destination_router_name);

        vector<Router> routers_at_previous_step = summary[timing_round - 1];
        Router intermediate_router_at_previous_step = routers_at_previous_step[intermediate_index];
        vector<vector<string>> inter_distance_table_at_prev = intermediate_router_at_previous_step.distance_table;
        string hopping_node_btw_intermediate_and_dest = "";
        
        string inter_to_dest_min_path_cost = min_path(inter_distance_table_at_prev, dest_index, router_count, hopping_node_btw_intermediate_and_dest);

        if(hopping_node_btw_intermediate_and_dest == this_router.name){
            // According to Split Horizon rule, do not consider the route which goes back to the destination router.
            return this_router.distance_table[row][col];
        }
        else if (inter_to_dest_min_path_cost == INFINITY_SYMBOL)
        {
            return INFINITY_SYMBOL;
        }
        else{
            intermediate_to_destination_cost = stoi(inter_to_dest_min_path_cost);
            result = source_to_intermediate_cost + intermediate_to_destination_cost;
        }

        return to_string(result);
    }
}

/**
 * Method for creating a distance table with the given router names
 * 
 * After this method, one of elements of the vectors <routers> will look like this:
 * 
 * [0]: " "  -> "Y"   -> "Z"
 * [1]: "Y" -> "INF" -> "INF"
 * [2]: "Z" -> "INF" -> "INF"
 * 
 * */
void initial(const vector<string>& router_names) {
    router_count = router_names.size();
    vector<string> tmp;

    for (int i = 0; i < router_count; i++) {
        tmp = router_names;
        routers.push_back(Router());
        routers[i].name = router_names[i];
        routers[i].distance_table.resize(router_count, vector<string>(router_count, INFINITY_SYMBOL));
        routers[i].distance_table[0][0] = " ";

        // Combining erase() and remove() to delete current Router out of the router vector, then resize the vector.
        tmp.erase(std::remove(tmp.begin(), tmp.end(), routers[i].name), tmp.end());

        int n = 1;
        for (int j = 0; j < (int) tmp.size(); j++) {
            string routerName = tmp[j];
            // Label destination node
            routers[i].distance_table[n][0] = routerName;

            // Label neighbour node
            routers[i].distance_table[0][n] = routerName;

            n++;
        }
    }
}

/**
 * Update the direct link costs.
 * 
 * Particulary, at this round of time, we only update distance cost to 
 * neighbouring nodes of a specific node (router).
 * 
 * For example: output value of one of distance tables will look like:
 * 
 * [0]: " " -> "Y"   -> "Z"
 * [1]: "Y" -> "2"   -> "INF"
 * [2]: "Z" -> "INF" -> "8"
 * 
 * The above is distance table of X, given that Y and Z are neighbours of X
 * */ 
void update_t0(const vector<string>& router_initial_cost) {
    vector<string> cost = router_initial_cost;
    vector<string> separate_cost;
    char delimiter = ' ';

    for (int i = 0; i < router_count; i++) {
        for (int j = 0; j < (int) cost.size(); j++) {
            separate_cost = split_string(cost[j], delimiter);

            string distance_cost_value = (separate_cost[2] != "-1") ? separate_cost[2] : INFINITY_SYMBOL;

            if (separate_cost[0] == routers[i].name) {
                int pos = get_location(routers[i].distance_table, separate_cost[1]);
                routers[i].distance_table[pos][pos] = distance_cost_value;
            }
            if (separate_cost[1] == routers[i].name) {
                int pos = get_location(routers[i].distance_table, separate_cost[0]);
                routers[i].distance_table[pos][pos] = distance_cost_value;
            }
        }
    }

    timing_round++;
    summary.push_back(routers);
}

/**
 * After getting the direct link cost, using the intermediate router to calculate the cost.
 * 
 * */ 
void update_t1() {
    for (int i = 0; i < router_count; i++) {
        for (int j = 0; j < router_count; j++) {
            for (int k = 0; k < router_count; k++) {
                if (routers[i].distance_table[j][k] == INFINITY_SYMBOL) {
                    routers[i].distance_table[j][k] = calculate(i, j, k);
                }
            }
        }
    }

    timing_round++;
    summary.push_back(routers);
    // display();
}

// Function to keep updating the distance tables until convergence is reached
void keep_update() {
    vector<Router> t2 = routers;
    for (int i = 0; i < router_count; i++) {
        for (int row = 1; row < router_count; row++) {
            for (int col = 1; col < router_count; col++) {
                if (row != col && routers[i].distance_table[row][col] != "-1") {
                    if (routers[i].distance_table[row][col] != calculate_optimised(i, row, col)) {
                        routers[i].distance_table[row][col] = calculate_optimised(i, row, col);
                    }
                }
            }
        }
    }
    
    if (t2 == routers) {
        // no changes of vector <routers>. Hence, stop the the loop of calling this method, i.e. keep_update()
        converged = true;
        t_index.push_back(timing_round);
    } else {
        converged = false;
        timing_round++;
        summary.push_back(routers);
    }
}

/**
 * Keep track of updated routes until reaching the convergence.
 * Particularly, this loop will end when no changes in all routing tables.
 * 
 * This method is called only once at the beginning to initialize the first stage
 * of all routing tables.
 * 
 * + Note: Each node (router) X has a routing table in which the optimised/least cost
 * to each destination node (from this node X) in the network is stored.
 * */
void convergence() {
    update_t1();
    while (!converged) {
        keep_update();
    }
}

// Function to update the distance tables with new link cost information
void update_distance_table(const vector<string>& update_info) {
    if (update_info.empty()) {
        return;
    }

    try {
        converged = false;
        for (int i = 0; i < router_count; i++) {
            for (int j = 0; j < (int) update_info.size(); j++) {
                vector<string> separate_inputs = split_string(update_info[j], ' ');
                if (separate_inputs[0] == routers[i].name) {
                    int dest_index = get_location(routers[i].distance_table, separate_inputs[1]);
                    routers[i].distance_table[dest_index][dest_index] = separate_inputs[2] != "-1" ? separate_inputs[2] : INFINITY_SYMBOL;
                }
                if (separate_inputs[1] == routers[i].name) {
                    int dest_index = get_location(routers[i].distance_table, separate_inputs[0]);
                    routers[i].distance_table[dest_index][dest_index] = separate_inputs[2] != "-1" ? separate_inputs[2] : INFINITY_SYMBOL;
                }
                for (int row = 1; row < router_count; row++) {
                    for (int col = 1; col < router_count; col++) {
                        if (row != col && routers[i].distance_table[row][col] != INFINITY_SYMBOL) {
                            if (routers[i].distance_table[row][col] != calculate_optimised(i, row, col)) {
                                routers[i].distance_table[row][col] = calculate_optimised(i, row, col);
                            }
                        }
                    }
                }
            }
        }
        summary.push_back(routers);
        timing_round++;

        while (!converged) {
            keep_update();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

// Function to generate the result for final routing table and print it
void print_output() {
    if (summary.empty()) {
        return;
    }

    int summary_size = summary.size();
    int index = 0;

    while (index < summary_size) {
        for (int i = 0; i < router_count; i++) {
            cout << summary[index][i].name << " Distance Table at t=" << index << "\n";
            for (int j = 0; j < router_count; j++) {
                for (int k = 0; k < router_count; k++) {
                    cout << summary[index][i].distance_table[j][k] << " ";
                }
                cout << endl;
            }
            cout << endl;
        }

        for (vector<int>::iterator it = t_index.begin(); it != t_index.end(); it++) {
            if (*it - 1 == index) {
                for (int i = 0; i < router_count; i++) {
                    cout << summary[index][i].name << " Routing Table:" << endl;
                    for (int j = 1; j < router_count; j++) {
                        string min_path_value = min_path(summary[index][i].distance_table, j, router_count);
                        string min_path_name_value = min_path_name(summary[index][i].distance_table, j, router_count);
                        if (min_path_value != INFINITY_SYMBOL && min_path_name_value != "original") {
                            cout << summary[index][i].distance_table[0][j] << "," << min_path_name_value << "," << min_path_value << endl;
                        }
                    }
                    cout << endl;
                }
            }
        }
        n++;
        index++;
    }
}


// Function is called after the UPDATE and END commands
void print_outputEnd()
{
    if (summary.empty())
    {
        return;
    }

    int summary_size = summary.size();

    for (int index = n; index < summary_size; index++)
    {
        for (const auto& router : summary[index])
        {
            cout << router.name << " Distance Table at t=" << index << "\n";
            for (const auto& row : router.distance_table)
            {
                for (const auto& cost : row)
                {
                    cout << cost << " ";
                }
                cout << endl;
            }
            cout << endl;

            if (find(t_index.begin(), t_index.end(), index + 1) != t_index.end())
            {
                cout << router.name << " Routing Table:" << endl;
                for (int j = 1; j < router_count; j++)
                {
                    const string& min_path_value = min_path(router.distance_table, j, router_count);
                    const string& min_path_name_value = min_path_name(router.distance_table, j, router_count);
                    if (min_path_value != "INF" && min_path_name_value != "original")
                    {
                        cout << router.distance_table[0][j] << "," << min_path_name_value << "," << min_path_value << endl;
                    }
                }
                cout << endl;
            }
        }
    }
}

// Function to take user input based on the expected input format
void take_input() {
    while (getline(cin, user_input)) {
        if (user_input == "DISTANCEVECTOR") {
            // Step 1: Populate size and node (router) names to the vector <routers>.
            if (user_inputs.empty()) {
                break;
            }
            initial(user_inputs);
            user_inputs.clear();
        } else if (user_input == "UPDATE") {
            // Step 2: Build intial routing table for each node (router) in the vector <routers>.
            if (user_inputs.empty()) {
                break;
            }
            update_t0(user_inputs);
            convergence();
            user_inputs.clear();
        } else if (user_input == "END") {
            // Step 3: Update direct connections of affected nodes (routers) and build new routing tables of all nodes.
            update_distance_table(user_inputs);
            print_output();
            return;
        } else {
            user_inputs.push_back(user_input);
        }
    }
}

// Main function
int main() {
    take_input();
    return 0;
}