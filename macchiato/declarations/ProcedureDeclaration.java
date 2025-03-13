package declarations;

import instructions.Block;

import java.util.*;

public class ProcedureDeclaration {
	private String name;

	private List<Character> arguments;

	private Block content;

	private ProcedureDeclaration() {}

	public ProcedureDeclaration(String name, List<Character> argumentsNames, Block content) throws Exception {
		this.name = name;

		if (argumentsNames == null) {
			this.arguments = null;
		}
		else {
			this.arguments = new ArrayList<>();
			this.arguments.addAll(argumentsNames);
		}

		this.content = content.deepCopy();
	}

	public ProcedureDeclaration deepCopy() throws Exception {
		ProcedureDeclaration res = new ProcedureDeclaration();

		res.name = name;

		res.arguments = new ArrayList<>();
		res.arguments.addAll(arguments);

		res.content = content.deepCopy();

		return res;
	}

	public String getName() {
		return name;
	}

	public Block getContentBlock() {
		return content;
	}

	public List<Character> getArguments() {
		return arguments;
	}

	public String getString() {
		StringBuilder res = new StringBuilder("void ");
		res.append(name);
		res.append('(');

		int i = 0;
		for (Character character : arguments) {
			res.append(character);

			if (i != arguments.size() - 1) {
				res.append(", ");
			}

			i++;
		}

		res.append(')');

		return res.toString();
	}
}
