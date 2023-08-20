#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#define BIG_INT 9999

using namespace std;

struct Router {
    string name;
    vector<vector<string>> distance_table;
};

static string user_input;
static vector<string> user_inputs;
static int router_count;
static bool converged = false;

static vector<Router> routers;
static int t = 0;
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
    if (source_to_intermediate_cost_s == "INF"){
        return "INF";
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
    if (intermediate_to_destination_cost_s == "INF"){
        return "INF";
    }

    intermediate_to_destination_cost = stoi(intermediate_to_destination_cost_s);
    result = source_to_intermediate_cost + intermediate_to_destination_cost;

    return to_string(result);
}

// Function to calculate the minimum cost path from a destination router to any other router 
// in the network
string min_path(const vector<vector<string>>& distance_table, int dest_index, int router_count) {
    const int tmp = BIG_INT;
    int min_cost = tmp;

    for (int i = 1; i < router_count; i++) {
        const string& cost = distance_table[dest_index][i];
        if (cost != "INF" && cost != "-1") {
            int tmp = stoi(cost);
            if (tmp < min_cost) {
                min_cost = tmp;
            }
        }
    }

    if (min_cost != BIG_INT) {
        return to_string(min_cost);
    }
    return "INF";
}

// Function to return the name of the router, corresponding to the minimum cost path
string min_path_name(const vector<vector<string>>& distance_table, int dest_index, int router_count) {
    const int tmp = BIG_INT;
    int min_cost = tmp;
    string result = "original";

    for (int i = 1; i < router_count; i++) {
        const string& cost = distance_table[dest_index][i];
        if (cost != "INF" && cost != "-1") {
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
    int index = 0;
    int result = 0;
    int source_to_intermediate_cost, intermediate_to_destination_cost = 0;

    const string& intermediate_router_name = routers[router_index].distance_table[0][col];
    const string& destination_router_name = routers[router_index].distance_table[row][0];

    if (routers[router_index].distance_table[col][col] == "-1" || routers[router_index].distance_table[col][col] == "INF") {
        return "INF";
    } else {
        source_to_intermediate_cost = stoi(routers[router_index].distance_table[col][col]);

        for (int j = 0; j < router_count; j++) {
            if (routers[j].name == intermediate_router_name) {
                index = j;
                break;
            }
        }

        int dest_index = get_location(routers[index].distance_table, destination_router_name);
        intermediate_to_destination_cost = stoi(min_path(summary[t - 1][index].distance_table, dest_index, router_count));
        result = source_to_intermediate_cost + intermediate_to_destination_cost;

        return to_string(result);
    }
}

// Function to initialize a distance table according to the given router names
void initial(const vector<string>& router_names) {
    router_count = router_names.size();
    vector<string> tmp;

    for (int i = 0; i < router_count; i++) {
        tmp = router_names;
        routers.push_back(Router());
        routers[i].name = router_names[i];
        routers[i].distance_table.resize(router_count, vector<string>(router_count, "INF"));
        routers[i].distance_table[0][0] = " ";

        tmp.erase(std::remove(tmp.begin(), tmp.end(), routers[i].name), tmp.end());

        int n = 1;
        for (int j = 0; j < (int) tmp.size(); j++) {
            string routerName = tmp[j];
            routers[i].distance_table[n][0] = routerName;
            routers[i].distance_table[0][n] = routerName;
            n++;
        }
    }
}

// Function to update the direct link costs in the initial stage (t=0)
void update_t0(const vector<string>& router_initial_cost) {
    vector<string> cost = router_initial_cost;
    vector<string> separate_cost;
    char delimiter = ' ';

    for (int i = 0; i < router_count; i++) {
        for (int j = 0; j < (int) cost.size(); j++) {
            separate_cost = split_string(cost[j], delimiter);

            string distance_cost_value = (separate_cost[2] == "-1") ? "INF" : separate_cost[2];

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

    t++;
    summary.push_back(routers);
}

// Function to update the distance tables at time step t=1.   
void update_t1() {
    for (int i = 0; i < router_count; i++) {
        for (int j = 0; j < router_count; j++) {
            for (int k = 0; k < router_count; k++) {
                if (routers[i].distance_table[j][k] == "INF") {
                    routers[i].distance_table[j][k] = calculate(i, j, k);
                }
            }
        }
    }

    t++;
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
        converged = true;
        t_index.push_back(t);
    } else {
        converged = false;
        t++;
        summary.push_back(routers);
    }
}

// Function to keep tracking the distance tables until convergence is reached
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
                    routers[i].distance_table[dest_index][dest_index] = separate_inputs[2] != "-1" ? separate_inputs[2] : "INF";
                }
                if (separate_inputs[1] == routers[i].name) {
                    int dest_index = get_location(routers[i].distance_table, separate_inputs[0]);
                    routers[i].distance_table[dest_index][dest_index] = separate_inputs[2] != "-1" ? separate_inputs[2] : "INF";
                }
                for (int row = 1; row < router_count; row++) {
                    for (int col = 1; col < router_count; col++) {
                        if (row != col && routers[i].distance_table[row][col] != "INF") {
                            if (routers[i].distance_table[row][col] != calculate_optimised(i, row, col)) {
                                routers[i].distance_table[row][col] = calculate_optimised(i, row, col);
                            }
                        }
                    }
                }
            }
        }
        summary.push_back(routers);
        t++;

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
                        if (min_path_value != "INF" && min_path_name_value != "original") {
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
            if (user_inputs.empty()) {
                break;
            }
            initial(user_inputs);
            user_inputs.clear();
        } else if (user_input == "UPDATE") {
            if (user_inputs.empty()) {
                break;
            }
            update_t0(user_inputs);
            convergence();
            user_inputs.clear();
        } else if (user_input == "END") {
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
