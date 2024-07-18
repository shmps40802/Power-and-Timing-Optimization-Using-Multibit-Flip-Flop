void Board::Banking(vector<vector<FlipFlop>> F1, vector<FlipFlop> F2) {
	int n = F1.size();
	for(int i = 0; i < n; i++) {
		string FlipFlopName = "C" + to_string(CellNumber);
		CellNumber++;
		NewFlipFlop.insert(FlipFlopName);
		F2[i].setInstName(FlipFlopName);
		vector<Point> curPin = F2[i].getPin();
		int d = 0, q = 0, c = 0;
		for(int j = 0; j < F1[i].size(); j++) {
			F1[i][j].display();
			vector<Point> PrevPin = F1[i][j].getPin();
			// add new flipflop
			if(NewFlipFlop.find(F1[i][j].getInstName()) != NewFlipFlop.end()) {
				NewFlipFlop.erase(F1[i][j].getInstName());
			}
			string prevcell = F1[i][j].getInstName();
			for(auto &p : PrevPin) {
				string prev = prevcell + "/" + p.name;
				string cur;
				// Pin mapping to Pin for 1 bit
				if(p.type == 'D') {
					while(curPin[d].type != 'D') {
						d++;
					}
					cur = FlipFlopName + "/" + curPin[d].name;
					map<string, bool> visited;
					float NS = F1[i][j].getSlack()[p.name];
					int fx = F2[i].getX() + curPin[d].x;
					int fy = F2[i].getY() + curPin[d].y;
					updateDSlack(prev, NS, fx, fy);
					F2[i].setSlack(curPin[d].name, NS);
					d++;
				}
				else if(p.type == 'Q') {
					while(curPin[q].type != 'Q') {
						q++;
					}
					cur = FlipFlopName + "/" + curPin[q].name;
					map<string, bool> visited;
					float WL = 0;
					int fx = F2[i].getX() + curPin[q].x;
					int fy = F2[i].getY() + curPin[q].y;
					updateQSlack(prev, visited, WL, fx, fy);
					q++;
				}
				else if(p.type == 'C') {
					while(curPin[c].type != 'C') {
						c++;
					}
					cur = FlipFlopName + "/" + curPin[c].name;
				}
				else {
					cout << prev << " not existing\n";
					break;
				}
				// modify net connection
				string netname = PointToNet[prev];
				Net[netname].erase(prev);
				Net[netname].insert(cur);
				PointToNet.erase(prev);
				PointToNet[cur] = netname;
			}
			// delete cell from location
			int fx = F1[i][j].getX();
			int fy = F1[i][j].getY();
			for(size_t k = 0; k < Location[fx][fy].size(); k++) {
				if(Location[fx][fy][k] == F1[i][j].getInstName()) {
					Location[fx][fy].erase(Location[fx][fy].begin() + i);
				}
			}
			InstToFlipFlop.erase(F1[i][j].getInstName());
		}
		F2[i].display();
		int x = F2[i].getX();
		int y = F2[i].getY();
		Location[x][y].push_back(FlipFlopName);
		InstToFlipFlop[FlipFlopName] = F2[i];
	}
}
