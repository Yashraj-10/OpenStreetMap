#include <bits/stdc++.h>
#include "rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

//Structure for storing nodes
typedef struct
{
    long long ID;
    double latitude, longitude;
    string name;
}Node;

//Structure for storing ways
typedef struct
{
    long long ID;
    vector<pair<long long, pair<double,double>>> path;
}Way;

//Func to check whether given string is substring of a node's name
bool check(string s1, string s2)
{
    transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
    transform(s2.begin(), s2.end(), s2.begin(), ::tolower);

    return (s1.find(s2) != string::npos);
}

//Func to sort the distance in k-nearest neighbours
bool inline comp(pair<double, long long> x, pair<double, long long> y) { return x.first < y.first; }


//Function to calculate distance using latitude and longitude using haversine formula
double haversine(double lat1, double lon1, double lat2, double lon2)
    {
        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLon = (lon2 - lon1) * M_PI / 180.0;
 
        lat1 = (lat1) * M_PI / 180.0;
        lat2 = (lat2) * M_PI / 180.0;
 
        double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
        double rad = 6371;
        double c = 2 * asin(sqrt(a));
        return rad*c;
    }

void Find_Node(vector<Node> ListOFnodes);
void Find_Nearest_Nodes(vector<Node> ListOFnodes);
int findNode(vector<Node> ListOFnodes, long long id);
void ShortestPath(vector<Node> ln, vector<Way> lw,long long id1,long long id2);
int minDistance(map<long long,double>dist, map<long long,bool> visited);

int main()
{
    //Declaring vectors for list of nodes and ways
    vector<Node> ListOfNodes;
    vector<Way> ListOfWays;

    //Initializing the OSM file
    file<> osmFile("map.osm");
    xml_document<> doc;
    doc.parse<0>(osmFile.data());
    xml_node<> *pRoot = doc.first_node(); // A pointer to the first node in OSM file

    //reading the OSM file
    Node n;
    xml_attribute<> *Attribute;
    int i=0;
    for(xml_node<> *node=pRoot->first_node("node"); node; node=node->next_sibling("node"))
    {
        Attribute = node->first_attribute("id");
        n.ID = stoll(Attribute->value());

        Attribute = node->first_attribute("lat");
        n.latitude = stod(Attribute->value());
        
        Attribute = node->first_attribute("lon");
        n.longitude = stod(Attribute->value());

        for(xml_node<> *child = node->first_node("tag"); child; child =child->next_sibling("tag"))
        {
            cout<<"y";
            Attribute = child->first_attribute("k");

            if(!Attribute)
                break;

            string name = Attribute->value();

            if(!name.compare("name"))
                n.name = child->first_attribute("v")->value();
        }
        
        ListOfNodes.push_back(n);
    }
    for (xml_node<> *WayNode = pRoot->first_node("way"); WayNode; WayNode = WayNode->next_sibling("way"))
    {
        cout<<i++<<endl;
        Way way_node;
        Attribute = WayNode->first_attribute("id");
        way_node.ID = stoll(Attribute->value());
        for (xml_node<> *child = WayNode->first_node("nd"); child; child = child->next_sibling("nd"))
        {
            Attribute = child->first_attribute("ref");
            int i = findNode(ListOfNodes, stoll(Attribute->value()));
            way_node.path.push_back({stoll(Attribute->value()), {ListOfNodes[i].latitude,ListOfNodes[i].longitude}});
        }
        ListOfWays.push_back(way_node);
    }
    int ch;
    //Menu for exploring the file
    do
    {
    printf("1] Print total number of nodes.\n");
    printf("2] Print total number of ways.\n");
    printf("3] Search a Node.\n");
    printf("4] Find k-closest Nodes to a Node.\n");
    printf("5] Find Shortest Distance between two Nodes.\n");
    printf("Choose an OPTION: ");
    int opt;
    scanf("%d",&opt);
    ch=opt;
    switch(opt)
    {
        case 1:
            printf("Number of Nodes are = %d.\n",ListOfNodes.size());
            break;
        case 2:
            printf("Number of Nodes are = %d.\n",ListOfWays.size());
            break;
        case 3:
            Find_Node(ListOfNodes);
            break;
        case 4:
            Find_Nearest_Nodes(ListOfNodes);
            break;
        case 5:
            long long id1,id2;
            cout<<"Enter first ID.\n";
            cin>>id1;
            cout<<"Enter the second ID.\n";
            cin>>id2;
            ShortestPath(ListOfNodes, ListOfWays, id1, id2);
            break;
        default:
            printf("Incorrect Option!!!");
    }
    } while (ch>=1 && ch<=5);
    return 0;
}

//Func to check whether a node is p[resent in the given list of nodes
void Find_Node(vector<Node> ListOFnodes)
{
    string str;
    printf("Enter the Name to be searched: ");
    cin>>str;
    for(auto N : ListOFnodes)
    {
        if(check(N.name,str))
        {
            cout<<"Node Found!!!"<<endl;
            cout<<"Details are: "<<endl;
            cout<<"Node ID: "<<N.ID<<endl;
            cout<<"Node Name: "<<N.name<<endl;
            cout<<"Node Latitude: "<<N.latitude<<endl;
            cout<<"Node Longitude: "<<N.longitude<<endl;
            return;
        }
    }
    printf("No Node is found with matching Name.");
}

//Function to find the index of a node with known id from the list of nodes
int findNode(vector<Node> ListOFnodes, long long id)
{
    int i;
    for (i = 0; i < ListOFnodes.size(); i++)
        if (ListOFnodes[i].ID == id)
            return i;

    return -1;
}


//Function to find the k-nearest neoghbours
void Find_Nearest_Nodes(vector<Node> ListOFnodes)
{
    int i;              //Loop var
    long long id;       //var to store id of the node goven by user
    printf("Enter the ID of nodes of the which the neighbours are to be found.\n");
    cin>>id;

    int posNode = findNode(ListOFnodes,id);

    if(posNode == -1)
    {
        cout<<"No node with given ID exists.";
        return;
    }

    vector<pair<double,long long>> distance;        //vector to store distance of a node from given node

    for (i = 0; i < ListOFnodes.size(); i++)
    {
        double length = haversine(ListOFnodes[i].latitude, ListOFnodes[i].longitude, ListOFnodes[posNode].latitude, ListOFnodes[posNode].longitude);
        distance.push_back({length, ListOFnodes[i].ID});
    }

    int k;
    cout<<"Enter the value of 'k' : ";
    cin>>k;

    if(k>=ListOFnodes.size())
    {
        printf("k can't be greater than the no of Nodes.\n");
        return;
    }

    sort(distance.begin(), distance.end(), comp);

    //printing the k-nearest neighbours
    printf("The k-nearest neighbours are\n");
    for(i=0;i<=k;i++)
    {
        cout<<distance[i].second<<"\n";
    }
}

//Func to find the
int minDistance(map<long long,double>dist, map<long long,bool> visited)
{
    double min = INT_MAX;
    int id;
    for(auto x:dist){
        if(!visited[x.first] && x.second<min) min=x.second,id=x.first;

    }
    return id;
}

//Function using dijsktra algo to find the shortest path between two nodes
void ShortestPath(vector<Node> ln, vector<Way> lw,long long id1,long long id2)
{
    map<long long, map<long long, double>> adjList; 
    int i;
    double d;

    for(Way w: lw)
    {
        for(i=0; i<w.path.size();i++)
        {
            d = haversine(w.path[i].second.first, w.path[i].second.second, w.path[i+1].second.first, w.path[i+1].second.second);
            adjList[w.path[i].first][w.path[i+1].first] = d;
            adjList[w.path[i+1].first][w.path[i].first] = d;
        }
    }

    long long V = adjList.size();
     map<long long,double>dist;
 
    map<long long,bool>visited;
    for(auto node: adjList){
       dist[node.first]=INT_MAX;
       visited[node.first]=false;
    }
 
    dist[id1] = 0;

    for (int count = 0; count < V - 1; count++) {
        int id = minDistance(dist, visited);
 
        visited[id] = true;
 
        for(auto node:adjList[id]){
            if(!visited[node.first] && dist[node.first]!=INT_MAX && dist[id]+node.second<dist[node.first])dist[node.first]=dist[id]+node.second;
        }
    }
    
    cout<<"Shortest Route has been found!!!\n";
    cout<<"The distance is: ";
    cout<<dist[id2]<<endl;
    
}