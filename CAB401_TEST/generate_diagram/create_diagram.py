from random import choice


def make_diagram(nodes=10,edges=5):

    with open(f"n_{nodes}_e{edges}_dotgraph.txt","w") as file:
        file.write("digraph someDotGraph {\n")
        file.write("splines=ortho;")
        nodes = list(range(nodes+edges))
        for node in nodes[-edges -1 :]:
            file.write(f'"node {node}" [fillcolor="red",style=filled,shape=polygon,sides=8];\n')
        for node in nodes[:-edges-1]:
            selected = []
            if node == 0: 
                file.write(f'"node {node}" [fillcolor="green",style=filled,shape=polygon,sides=8];\n')
                file.write('"node {}" -> "node {}";\n'.format(node,node+1))
                continue
            file.write('"node {}" -> "node {}";\n'.format(node,node+1))
            for edge in range(edges-1):
                new_edge = choice(nodes)
                while new_edge in selected or new_edge < node:
                    new_edge = choice(nodes)
                file.write('"node {}" -> "node {}";\n'.format(node,new_edge))
                selected.append(new_edge)
        file.write("}\n")

if __name__ == "__main__":
    make_diagram(nodes=25,edges=7)

# debugging line to run on this file
# -Tpng C:\Users\adam\Desktop\graphviz_tests\generate_diagram\diagram.txt -o C:\Users\adam\Desktop\graphviz_tests\generate_diagram\output.png